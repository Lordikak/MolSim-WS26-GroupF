#pragma once

#include <functional>
#include <memory>

#include "utils/MaxwellBoltzmannDistribution.h"
#include "utils/ParticleData.h"

struct ParticleGenerator {
    virtual ~ParticleGenerator() = default;
    virtual void ForEach(const std::function<void(const ParticleData &)> &call_back) const = 0;
};

using GeneratorVector = std::vector<std::unique_ptr<ParticleGenerator>>;

/**
 * @brief initialize a molecule cuboid
 *
 * contains the lower left most particle and each dimension, as well as
 * the space between each particle
 *
 * overloads an iterator with callback for particle initialization
 */
struct ParticleCuboid : ParticleGenerator {
    Vec3 min;
    Vec3 initial_velocity;
    std::array<std::size_t, 3> dimensions;
    double spacing;
    double mass;

    double brownian_velocity;

    void ForEach(const std::function<void(const ParticleData &)> &call_back) const override {
        for (std::size_t i = 0; i < dimensions[0]; ++i) {
            for (std::size_t j = 0; j < dimensions[1]; ++j) {
                for (std::size_t k = 0; k < dimensions[2]; ++k) {
                    Vec3 position{min[0] + static_cast<double>(i) * spacing,
                                  min[1] + static_cast<double>(j) * spacing,
                                  min[2] + static_cast<double>(k) * spacing};

                    auto random_velocity =
                        maxwellBoltzmannDistributedVelocity(brownian_velocity, 2);

                    Vec3 velocity = initial_velocity + random_velocity;

                    call_back(ParticleData{position, velocity, mass});
                }
            }
        }
    }
};

/**
 * @brief initialize a cylindrical molecule
 *
 * constains the location of the center of the lower round face, its height
 */
struct ParticleDisc : ParticleGenerator {
    Vec3 center;
    std::size_t height;
    Vec3 initial_velocity;
    std::size_t radius;
    double spacing;
    double mass;

    double brownian_velocity;

    void ForEach(const std::function<void(const ParticleData &)> &call_back) const override {
        // calculate and save the radius squared distance. Avoid overflows
        const long long r2 = radius * radius;

        const int r = radius;

        // consider a square of length 2 * radius
        for (int i = -r; i <= r; ++i) {
            for (int j = -r; j <= r; ++j) {
                for (std::size_t k = 0; k < height; ++k) {
                    // if distance from center leq to radius, initialize
                    // avoid sqrt
                    if (i * i + j * j <= r2) {
                        Vec3 position{center[0] + static_cast<double>(i) * spacing,
                                      center[1] + static_cast<double>(j) * spacing,
                                      center[2] + static_cast<double>(k) * spacing};

                        auto random_velocity =
                            maxwellBoltzmannDistributedVelocity(brownian_velocity, 2);

                        Vec3 velocity = initial_velocity + random_velocity;

                        call_back(ParticleData{position, velocity, mass});
                    }
                }
            }
        }
    }
};

/**
 * @brief compatibility method to support single particle initialization from txt file
 *
 * generates a single particle with a position, velocity, and mass
 */
struct ParticleSingleton : ParticleGenerator {
    ParticleData pd;
};
