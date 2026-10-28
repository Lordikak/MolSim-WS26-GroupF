#pragma once
#include <functional>

#include "ParticleContainer.h"

namespace simulation {
struct SimulationSettings {
    double startTime;
    double endTime;
    double deltaT;
};

void RunSimulation(
    ParticleContainer &particles, const struct SimulationSettings settings,
    const std::function<void(const ParticleContainer &, int, double)> &post_iteration);
}  // namespace simulation
