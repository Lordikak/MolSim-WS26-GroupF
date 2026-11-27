#include "JsonReader.h"

#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

#include "ParticleContainer.h"
#include "utils/MaxwellBoltzmannDistribution.h"

namespace file_reader {
namespace {
using Json = nlohmann::json;

Vec3 ReadVec3(const Json &value, const std::string &field_name) {
    if (!value.is_array() || value.size() != 3) {
        throw std::invalid_argument("'" + field_name + "' must be an array of exactly 3 numbers");
    }

    Vec3 result{};
    for (std::size_t i = 0; i < 3; ++i) {
        result[i] = value.at(i).get<double>();
        if (!std::isfinite(result[i])) {
            throw std::invalid_argument("'" + field_name + "' values must be finite");
        }
    }

    return result;
}

std::size_t ReadPositiveCount(const Json &value, const std::string &field_name) {
    if (!value.is_number_integer()) {
        throw std::invalid_argument("'" + field_name + "' must be a positive integer");
    }

    // Read as a signed value first so negative counts are rejected.
    const long long count = value.get<long long>();
    if (count <= 0) {
        throw std::invalid_argument("'" + field_name + "' must be greater than zero");
    }

    return static_cast<std::size_t>(count);
}

double ReadRequiredDouble(const Json &value, const std::string &field_name) {
    if (!value.is_number_float()) {
        throw std::invalid_argument("'" + field_name + "' must be a positive scalar double");
    }

    // Read the scalar value and attempt to cast to integer
    try {
        const double val = value.get<double>();
        if (val <= 0) {
            throw std::invalid_argument("'" + field_name + "' must be greater than zero");
        }

        return val;

    } catch (const Json::parse_error &e) {
        throw std::invalid_argument("'" + field_name + "' must be a string" +
                                    std::string(e.what()));
    }
}

/**
 * @brief internal parsing and validation for an expected double value
 * @param value the current parsed yaml node
 * @param field_name the name of the parsed field
 * @param default_value user_defined value read from command line.
 * Used as a fallback value for if the read file does not contain it
 * @return either the value written in the field or the default provided
 */
double ReadPositiveDoubleAgainstDefault(const Json &value, const std::string &field_name,
                                        const double default_value) {
    // if the field is not in the file, return the default
    if (value.is_null()) {
        return default_value;
    }
    // if the field is present but malformed, complain
    if (!value.is_number_float()) {
        throw std::invalid_argument("'" + field_name + "' must be a scalar");
    }

    try {
        const double val = value.get<double>();
        if (val <= 0.0) {
            throw std::invalid_argument("'" + field_name + "' must be a positive double");
        }

        return static_cast<double>(val);
    } catch (const Json::parse_error &e) {
        throw std::invalid_argument("'" + field_name + "' must be a string" +
                                    std::string(e.what()));
    }
}

/**
 * @brief internal enum parsing and conversion method to avoid overloading ::convert
 * @param value the current parsed yaml node
 * @param field_name the name of the parsed field
 * @return the appropriate enum representing the behavior of that boundary face
 */
BoundaryCondition ConvertFromScalar(const Json &value, const std::string &field_name) {
    if (!value.is_string()) {
        throw std::invalid_argument("'" + field_name + "' must be an appropriate string");
    }

    const auto cond = value.get<std::string>();

    if (cond == "outflow") {
        return BoundaryCondition::kOutflow;
    } else if (cond == "reflecting") {
        return BoundaryCondition::kReflecting;
    } else {
        throw std::invalid_argument("'" + field_name + "' must either be outflow or reflecting");
    }
}

std::array<BoundaryCondition, 6> ReadBoundaryConditions(const Json &value,
                                                        const std::string &field_name) {
    constexpr std::array<std::string_view, 6> kBoundaryKeys = {"min_x", "max_x", "min_y",
                                                               "max_y", "min_z", "max_z"};

    std::array<BoundaryCondition, 6> result;

    for (std::size_t i = 0; i < kBoundaryKeys.size(); ++i) {
        const auto node = value.at(std::string(kBoundaryKeys[i]));
        std::string key(kBoundaryKeys[i]);

        if (node.is_null()) {
            throw std::invalid_argument("'" + field_name + "' missing from input file");
        }

        if (!node.is_string()) {
            throw std::invalid_argument("'" + field_name + "' must be an appropriate string");
        }

        result[i] = ConvertFromScalar(node, field_name + "." + key);
    }

    return result;
}

ParticleCuboid ParseCuboid(const Json &cuboid) {
    ParticleCuboid pc;

    const Vec3 origin = ReadVec3(cuboid.at("origin"), "origin");
    const Vec3 initial_velocity = ReadVec3(cuboid.at("initial_velocity"), "initial_velocity");

    const auto &counts = cuboid.at("particle_counts");
    if (!counts.is_array() || counts.size() != 3) {
        throw std::invalid_argument("'particle_counts' must be an array of exactly 3 integers");
    }
    const std::size_t n1 = ReadPositiveCount(counts[0], "particle_counts[0]");
    const std::size_t n2 = ReadPositiveCount(counts[1], "particle_counts[1]");
    const std::size_t n3 = ReadPositiveCount(counts[2], "particle_counts[2]");

    const double spacing = ReadRequiredDouble(cuboid.at("spacing"), "spacing");
    const double mass = ReadRequiredDouble(cuboid.at("mass"), "mass");
    const double brownian_velocity =
        ReadRequiredDouble(cuboid.at("brownian_velocity"), "brownian_velocity");

    SPDLOG_DEBUG("Cuboid at {}: {}x{}x{} particles, h = {}, m = {}", origin, n1, n2, n3, spacing,
                 mass);

    // initialize the particle cuboid and return
    pc.min = origin;
    pc.initial_velocity = initial_velocity;
    pc.dimensions = {n1, n2, n3};
    pc.spacing = spacing;
    pc.mass = mass;
    pc.brownian_velocity = brownian_velocity;

    return pc;
}

ParticleDisc ParseDisc(const Json &disc) {
    ParticleDisc pd;

    const Vec3 origin = ReadVec3(disc.at("origin"), "origin");
    const Vec3 initial_velocity = ReadVec3(disc.at("initial_velocity"), "initial_velocity");

    const std::size_t radius = ReadPositiveCount(disc.at("radius"), "radius");
    const std::size_t height = ReadPositiveCount(disc.at("height"), "height");

    const double spacing = ReadRequiredDouble(disc.at("spacing"), "spacing");
    const double mass = ReadRequiredDouble(disc.at("mass"), "mass");
    const double brownian_velocity =
        ReadRequiredDouble(disc.at("brownian_velocity"), "brownian_velocity");

    SPDLOG_DEBUG("Disc centered at {}: radius {}, h = {}, m = {}", origin, radius, spacing, mass);

    pd.center = origin;
    pd.height = height;
    pd.initial_velocity = initial_velocity;
    pd.radius = radius;
    pd.spacing = spacing;
    pd.mass = mass;
    pd.brownian_velocity = brownian_velocity;

    return pd;
}

}  // namespace

GeneratorVector JsonReader::InitializeGenerators(const Json &value) {
    GeneratorVector generators;

    const auto &cuboids = value.at("cuboids");
    const auto &discs = value.at("discs");

    if (cuboids.contains("cuboids")) {
        if (!cuboids.is_array()) {
            throw std::invalid_argument("'cuboids' must be an array");
        }

        SPDLOG_DEBUG("Reading {} cuboids from {}", cuboids.size(), file_path_);

        for (const auto &cuboid : cuboids) {
            generators.push_back(std::make_unique<ParticleCuboid>(ParseCuboid(cuboid)));
        }
    }

    if (discs.contains("discs")) {
        if (!discs.is_array()) {
            throw std::invalid_argument("'discs' must be an array");
        }

        SPDLOG_DEBUG("Reading {} discs from {}", discs.size(), file_path_);

        for (const auto &disc : discs) {
            generators.push_back(std::make_unique<ParticleDisc>(ParseDisc(disc)));
        }
    }

    return generators;
}

SimulationSpec JsonReader::Read(SimulationSettings user_defined) {
    std::ifstream input_file(file_path_);
    if (!input_file) {
        throw std::invalid_argument("Could not open file: " + file_path_);
    }

    const Json config = Json::parse(input_file);

    if (config.at("version").get<int>() != 1) {
        throw std::invalid_argument("Unsupported JSON config version");
    }

    // initialize generators for each molecule
    auto generators = InitializeGenerators(config);

    // read domain boundary behavior
    const auto behaviors =
        ReadBoundaryConditions(config.at("boundary_conditions"), "boundary_conditions");

    // read domain bounds and boundary conditions
    const Vec3 min_bounds = ReadVec3(config.at("bounds_minimum"), "bounds_minimum");
    const Vec3 max_bounds = ReadVec3(config.at("bounds_maximum"), "bounds_maximum");

    // read cutoff
    const double cutoff = ReadRequiredDouble(config.at("cutoff"), "cutoff");

    // read simulation run configuration settings
    const double start =
        ReadPositiveDoubleAgainstDefault(config.at("start"), "start", user_defined.start_time);
    const double end =
        ReadPositiveDoubleAgainstDefault(config.at("end"), "end", user_defined.end_time);
    if (end >= start) {
        throw std::invalid_argument("'end' must be a positive double greater than the start time");
    }
    const double delta_t =
        ReadPositiveDoubleAgainstDefault(config.at("delta_t"), "delta_t", user_defined.delta_t);
    if (delta_t > end - start) {
        throw std::invalid_argument("'delta_t' must be less than the duration of the simulation");
    }

    SimulationSettings ss = SimulationSettings(start, end, delta_t);
    DomainSettings ds = DomainSettings(min_bounds, max_bounds, cutoff, behaviors);
    SimulationSpec spec = SimulationSpec(ss, ds, std::move(generators));

    return spec;
}

}  // namespace file_reader
