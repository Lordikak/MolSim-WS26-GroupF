#include "Simulation.h"

#include "LennardJones.h"
#include "utils/ArrayUtils.h"

namespace simulation {
namespace {

void CalculateX(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetX(p.x() + delta_t * p.v() + (delta_t * delta_t / (2 * p.m())) * p.f());
    }
}

/**
 * @brief Computes LJ for each particle pair
 *
 * computed once per pair and then applies to both using newtons third law: F_ij = - F_ji
 *
 * @param particles particles to compute the forces for
 */
void CalculateF(ParticleContainer &particles) {
    // store old force in old_f and reset it, as it is calculated in the next step
    for (auto p : particles) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    particles.ApplyPairForces(LennardJones{});
}

void CalculateV(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetV(p.v() + delta_t * (p.f() + p.old_f()) / (2 * p.m()));
    }
}
}  // namespace

void RunSimulation(
    ParticleContainer &particles, const SimulationSettings settings,
    const std::function<void(const ParticleContainer &, int, double)> &post_iteration) {
    double current_time = settings.start_time;

    int iteration = 0;

    CalculateF(particles);

    // for this loop, we assume: current x, current f and current v are known
    while (current_time < settings.end_time) {
        CalculateX(particles, settings.delta_t);
        CalculateF(particles);
        CalculateV(particles, settings.delta_t);

        iteration++;
        current_time += settings.delta_t;
        post_iteration(particles, iteration, current_time);
    }
}
}  // namespace simulation
