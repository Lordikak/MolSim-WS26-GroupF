#pragma once
#include <functional>

#include "ParticleContainer.h"

namespace simulation {
struct SimulationSettings {
    double start_time;
    double end_time;
    double delta_t;
};

void RunSimulation(
    ParticleContainer &particles, const SimulationSettings settings,
    const std::function<void(const ParticleContainer &, int, double)> &post_iteration);
}  // namespace simulation
