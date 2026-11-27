#include "YamlReader.h"

#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "ParticleContainer.h"
#include "SimulationConfig.h"
#include "utils/BoundaryFace.h"
#include "utils/MaxwellBoltzmannDistribution.h"

namespace file_reader {
namespace {

Vec3 ReadVec3(const YAML::Node &value, const std::string &field_name) {
    if (!value.IsSequence() || value.size() != 3) {
        throw std::invalid_argument("'" + field_name + "' must be an array of exactly 3 numbers");
    }

    Vec3 result{};
    for (std::size_t i = 0; i < 3; ++i) {
        result[i] = value[i].as<double>();
        if (!std::isfinite(result[i])) {
            throw std::invalid_argument("'" + field_name + "' values must be finite");
        }
    }

    return result;
}

std::size_t ReadPositiveCount(const YAML::Node &value, const std::string &field_name) {
    if (!value.IsScalar()) {
        throw std::invalid_argument("'" + field_name + "' must be a positive scalar integer");
    }

    // Read the scalar value and attempt to cast to integer
    try {
        const long long count = value.as<long long>();
        if (count <= 0) {
            throw std::invalid_argument("'" + field_name + "' must be greater than zero");
        }

        return static_cast<std::size_t>(count);
    } catch (const YAML::BadConversion &e) {
        throw std::invalid_argument("'" + field_name + "' must be a string" +
                                    std::string(e.what()));
    }
}

double ReadPositiveRequiredDouble(const YAML::Node &value, const std::string &field_name) {
    if (!value.IsScalar()) {
        throw std::invalid_argument("'" + field_name + "' must be a positive scalar double");
    }

    // Read the scalar value and attempt to cast to integer
    try {
        const double val = value.as<double>();
        if (val <= 0) {
            throw std::invalid_argument("'" + field_name + "' must be a positive double");
        }

        return val;

    } catch (const YAML::BadConversion &e) {
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
double ReadNNDoubleAgainstDefault(const YAML::Node &value, const std::string &field_name,
                                  const double default_value) {
    // if the field is not in the file, return the default
    if (!value.IsDefined() || value.IsNull()) {
        return default_value;
    }
    // if the field is present but malformed, complain
    if (!value.IsScalar()) {
        throw std::invalid_argument("'" + field_name + "' must be a scalar");
    }

    try {
        const double val = value.as<double>();
        if (val < 0.0) {
            throw std::invalid_argument("'" + field_name + "' must be a non-negative double");
        }

        return static_cast<double>(val);
    } catch (const YAML::BadConversion &e) {
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
BoundaryCondition ConvertFromScalar(const YAML::Node &value, const std::string &field_name) {
    if (!value.IsScalar()) {
        throw std::invalid_argument("'" + field_name + "' must be an appropriate string");
    }

    const auto cond = value.as<std::string>();

    if (cond == "outflow") {
        return BoundaryCondition::kOutflow;
    } else if (cond == "reflecting") {
        return BoundaryCondition::kReflecting;
    } else {
        throw std::invalid_argument("'" + field_name + "' must either be outflow or reflecting");
    }
}

std::array<BoundaryCondition, 6> ReadBoundaryConditions(const YAML::Node &value,
                                                        const std::string &field_name) {
    constexpr std::array<std::string_view, 6> kBoundaryKeys = {"min_x", "max_x", "min_y",
                                                               "max_y", "min_z", "max_z"};

    std::array<BoundaryCondition, 6> result;

    for (std::size_t i = 0; i < kBoundaryKeys.size(); ++i) {
        const auto node = value[std::string(kBoundaryKeys[i])];
        std::string key(kBoundaryKeys[i]);

        if (!node.IsDefined() || node.IsNull()) {
            throw std::invalid_argument("'" + field_name + "' missing from input file");
        }

        if (!node.IsScalar()) {
            throw std::invalid_argument("'" + field_name + "' must be a scalar");
        }

        result[i] = ConvertFromScalar(node, field_name + "." + key);
    }

    return result;
}

ParticleCuboid ParseCuboid(const YAML::Node &cuboid) {
    ParticleCuboid pc;

    const Vec3 origin = ReadVec3(cuboid["origin"], "origin");
    const Vec3 initial_velocity = ReadVec3(cuboid["initial_velocity"], "initial_velocity");

    const auto &counts = cuboid["dimensions"];
    if (!counts.IsSequence() || counts.size() != 3) {
        throw std::invalid_argument("'dimensions' must be an array of exactly 3 integers");
    }
    const std::size_t n1 = ReadPositiveCount(counts[0], "dimensions[0]");
    const std::size_t n2 = ReadPositiveCount(counts[1], "dimensions[1]");
    const std::size_t n3 = ReadPositiveCount(counts[2], "dimensions[2]");

    const double spacing = ReadPositiveRequiredDouble(cuboid["spacing"], "spacing");
    const double mass = ReadPositiveRequiredDouble(cuboid["mass"], "mass");
    const double brownian_velocity =
        ReadPositiveRequiredDouble(cuboid["brownian_velocity"], "brownian_velocity");

    SPDLOG_DEBUG("Cuboid at {}: {}x{}x{} particles, h = {}, m = {}", origin, n1, n2, n3, spacing,
                 mass);

    // initialize the particle cuboid and return
    pc.min = origin;
    pc.dimensions = {n1, n2, n3};
    pc.spacing = spacing;
    pc.mass = mass;
    pc.initial_velocity = initial_velocity;
    pc.brownian_velocity = brownian_velocity;

    return pc;
}

ParticleDisc ParseDisc(const YAML::Node &disc) {
    ParticleDisc pd;

    const Vec3 origin = ReadVec3(disc["origin"], "origin");
    const Vec3 initial_velocity = ReadVec3(disc["initial_velocity"], "initial_velocity");

    const std::size_t radius = ReadPositiveCount(disc["radius"], "radius");
    const std::size_t height = ReadPositiveCount(disc["height"], "height");

    const double spacing = ReadPositiveRequiredDouble(disc["spacing"], "spacing");
    const double mass = ReadPositiveRequiredDouble(disc["mass"], "mass");
    const double brownian_velocity =
        ReadPositiveRequiredDouble(disc["brownian_velocity"], "brownian_velocity");

    SPDLOG_DEBUG("Disc centered at {}: radius {}, h = {}, m = {}", origin, radius, spacing, mass);

    pd.center = origin;
    pd.radius = radius;
    pd.height = height;
    pd.spacing = spacing;
    pd.mass = mass;
    pd.initial_velocity = initial_velocity;
    pd.brownian_velocity = brownian_velocity;

    return pd;
}

}  // namespace

GeneratorVector YamlReader::InitializeGenerators(const YAML::Node &value) {
    GeneratorVector generators;

    const auto &cuboids = value["cuboids"];
    const auto &discs = value["discs"];

    if (cuboids.IsDefined()) {
        if (!cuboids.IsSequence()) {
            throw std::invalid_argument("'cuboids' must be a sequence");
        }

        SPDLOG_DEBUG("Reading {} cuboids from {}", cuboids.size(), file_path_);

        for (const auto &cuboid : cuboids) {
            generators.push_back(std::make_unique<ParticleCuboid>(ParseCuboid(cuboid)));
        }
    }

    if (discs.IsDefined()) {
        if (!discs.IsSequence()) {
            throw std::invalid_argument("'discs' must be a sequence");
        }

        SPDLOG_DEBUG("Reading {} discs from {}", discs.size(), file_path_);

        for (const auto &disc : discs) {
            generators.push_back(std::make_unique<ParticleDisc>(ParseDisc(disc)));
        }
    }

    return generators;
}

SimulationSpec YamlReader::Read(SimulationSettings user_defined) {
    const YAML::Node config = YAML::LoadFile(file_path_);

    if (config["version"].as<int>() != 1) {
        throw std::invalid_argument("Unsupported YAML config version");
    }

    // initialize generators for each molecule
    auto generators = InitializeGenerators(config);

    // read domain boundary behavior
    const auto behaviors =
        ReadBoundaryConditions(config["boundary_conditions"], "boundary_conditions");

    // read domain bounds and boundary conditions
    const Vec3 min_bounds = ReadVec3(config["bounds_minimum"], "bounds_minimum");
    const Vec3 max_bounds = ReadVec3(config["bounds_maximum"], "bounds_maximum");

    // read cutoff
    const double cutoff = ReadPositiveRequiredDouble(config["cutoff"], "cutoff");

    // read simulation run configuration settings
    const double start =
        ReadNNDoubleAgainstDefault(config["start"], "start", user_defined.start_time);
    const double end = ReadNNDoubleAgainstDefault(config["end"], "end", user_defined.end_time);
    if (end <= 0 || end <= start) {
        throw std::invalid_argument("'end' must be a positive double greater than the start time");
    }
    const double delta_t =
        ReadNNDoubleAgainstDefault(config["delta_t"], "delta_t", user_defined.delta_t);
    if (delta_t > end - start) {
        throw std::invalid_argument("'delta_t' must be less than the duration of the simulation");
    }

    SimulationSettings ss = SimulationSettings(start, end, delta_t);
    DomainSettings ds = DomainSettings(min_bounds, max_bounds, cutoff, behaviors);
    SimulationSpec spec = SimulationSpec(ss, ds, std::move(generators));

    return spec;
}
}  // namespace file_reader
