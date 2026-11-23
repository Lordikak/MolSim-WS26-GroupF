#include "YamlReader.h"

#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include "ParticleContainer.h"
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

void GenerateParticlesInCuboid(ParticleContainer &particles, const YAML::Node &cuboid) {
    const Vec3 origin = ReadVec3(cuboid["origin"], "origin");
    const Vec3 initial_velocity = ReadVec3(cuboid["initial_velocity"], "initial_velocity");

    const auto &counts = cuboid["particle_counts"];
    if (!counts.IsSequence() || counts.size() != 3) {
        throw std::invalid_argument("'particle_counts' must be an array of exactly 3 integers");
    }
    const std::size_t n1 = ReadPositiveCount(counts[0], "particle_counts[0]");
    const std::size_t n2 = ReadPositiveCount(counts[1], "particle_counts[1]");
    const std::size_t n3 = ReadPositiveCount(counts[2], "particle_counts[2]");

    const double spacing = cuboid["spacing"].as<double>();
    if (!std::isfinite(spacing) || spacing <= 0.0) {
        throw std::invalid_argument("'spacing' must be finite and greater than zero");
    }

    const double mass = cuboid["mass"].as<double>();
    if (!std::isfinite(mass) || mass <= 0.0) {
        throw std::invalid_argument("'mass' must be finite and greater than zero");
    }

    const double average_velocity = cuboid["average_velocity"].as<double>();
    if (!std::isfinite(average_velocity) || average_velocity < 0.0) {
        throw std::invalid_argument("'average_velocity' must be equal or greater than zero");
    }

    SPDLOG_DEBUG("Cuboid at {}: {}x{}x{} particles, h = {}, m = {}", origin, n1, n2, n3, spacing,
                 mass);

    for (std::size_t i = 0; i < n1; ++i) {
        for (std::size_t j = 0; j < n2; ++j) {
            for (std::size_t k = 0; k < n3; ++k) {
                Vec3 position{origin[0] + static_cast<double>(i) * spacing,
                              origin[1] + static_cast<double>(j) * spacing,
                              origin[2] + static_cast<double>(k) * spacing};

                auto random_velocity = maxwellBoltzmannDistributedVelocity(average_velocity, 2);

                Vec3 velocity = initial_velocity + random_velocity;

                particles.add(position, velocity, mass);
            }
        }
    }
}

void GenerateParticlesInDisc(ParticleContainer &particles, const YAML::Node &disc) {
    const Vec3 origin = ReadVec3(disc["origin"], "origin");
    const Vec3 initial_velocity = ReadVec3(disc["initial_velocity"], "initial_velocity");

    const double radius = disc["radius"].as<double>();

    if (radius <= 0.0) {
        throw std::invalid_argument("'radius' must be a positive double");
    }

    const std::size_t height = disc["height"].as<int>();
    if (!std::isfinite(height) || height <= 0) {
        throw std::invalid_argument("'height' must be a positive integer");
    }

    const double spacing = disc["spacing"].as<double>();
    if (!std::isfinite(spacing) || spacing <= 0.0) {
        throw std::invalid_argument("'spacing' must be finite and greater than zero");
    }

    const double mass = disc["mass"].as<double>();
    if (!std::isfinite(mass) || mass <= 0.0) {
        throw std::invalid_argument("'mass' must be finite and greater than zero");
    }

    const double average_velocity = disc["average_velocity"].as<double>();
    if (!std::isfinite(average_velocity) || average_velocity < 0.0) {
        throw std::invalid_argument("'average_velocity' must be equal or greater than zero");
    }

    SPDLOG_DEBUG("Disc centered at {}: radius {}, h = {}, m = {}", origin, radius, spacing, mass);

    // calculate and save the radius squared distance. Avoid overflows
    const long long r2 = radius * radius;

    // consider a square of length 2 * radius
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            for (std::size_t k = 0; k < height; ++k) {
                // if distance from center leq to radius, initialize
                // avoid sqrt
                if (i * i + j * j <= r2) {
                    Vec3 position{origin[0] + static_cast<double>(i) * spacing,
                                  origin[1] + static_cast<double>(j) * spacing,
                                  origin[2] + static_cast<double>(k) * spacing};

                    auto random_velocity = maxwellBoltzmannDistributedVelocity(average_velocity, 2);

                    Vec3 velocity = initial_velocity + random_velocity;

                    particles.add(position, velocity, mass);
                }
            }
        }
    }
}

// void initialize_faces(ParticleContainer &particles, const YAML::Node &config) {
//
// }
//
// }

}  // namespace

void YamlReader::Read(ParticleContainer &particles) {
    const YAML::Node config = YAML::LoadFile(file_path_);

    if (config["version"].as<int>() != 1) {
        throw std::invalid_argument("Unsupported YAML config version");
    }

    const auto &cuboids = config["cuboids"];
    if (!cuboids.IsSequence()) {
        throw std::invalid_argument("'cuboids' must be an array");
    }

    SPDLOG_DEBUG("Reading {} cuboids from {}", cuboids.size(), file_path_);

    for (const auto &cuboid : cuboids) {
        GenerateParticlesInCuboid(particles, cuboid);
    }

    const auto &discs = config["discs"];
    if (!discs.IsSequence()) {
        throw std::invalid_argument("'discs' must be an array");
    }

    SPDLOG_DEBUG("Reading {} discs from {}", discs.size(), file_path_);

    for (const auto &disc : discs) {
        GenerateParticlesInDisc(particles, disc);
    }

    // read cutoff
    if (config["cutoff"].as<double>() <= 0.0) {
        throw std::invalid_argument("'cutoff' must be greater than zero");
    }

    // read domain bounds and boundary conditions
    Vec3 min_bounds = ReadVec3(discs["bounds_minimum"], "bounds_minimum");
    Vec3 max_bounds = ReadVec3(discs["bounds_maximum"], "bounds_maximum");
}
}  // namespace file_reader
