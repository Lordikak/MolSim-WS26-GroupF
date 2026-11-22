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
    } catch (const YAML::BadConversion) {
        throw std::invalid_argument("'" + field_name + "' must be a string");
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
}
}  // namespace file_reader
