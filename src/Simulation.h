#pragma once
#include <functional>

#include "ParticleContainer.h"

/**
 * @brief configuration parameters for a simulation run
 * 
 * contains positive arguments start time, end time, and delta t
 */
namespace simulation {
struct SimulationSettings {
    double start_time;
    double end_time;
    double delta_t;
};

/**
 * @brief Calculates each iteration of a simulation run
 * 
 * calculates forces, positions, and velocities for each particle each iteration
 * beginning at settings.start_time until settings.end_time, with a step size
 * settings.delta_t
 * @param &particles Particle system to simulate
 * @param settings struct of double arguments start time, end time, and delta t
 * @param &post_iteration callback invoked after each iteration
 *          Arguments-
 *          current state
 *          iteration count
 *          current time
 */
void RunSimulation(
    ParticleContainer &particles, const SimulationSettings settings,
    const std::function<void(const ParticleContainer &, int, double)> &post_iteration);
}  // namespace simulation
