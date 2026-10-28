#include "Simulation.h"

#include "utils/ArrayUtils.h"

namespace simulation {
void CalculateX(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetX(p.x() + delta_t * p.v() + (delta_t * delta_t / (2 * p.m())) * p.f());
    }
}

/**
 * @brief Computes gravitational force for every particle
 *
 * F_ij = m_i * m_j / |x_i - x_j|^3 * (x_j - x_i)
 * computed once per pair and then applies to both using newtons third law: F_ij = - F_ji
 * which halves the work
 *
 * @param particles particles to compute the forces for
 */
void CalculateF(ParticleContainer &particles) {
    // store old force in old_f and reset it, as it is calculated in the next step
    for (auto p : particles) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    particles.ApplyPairForces([](auto p_i, auto p_j) {
        const Vec3 delta_x = p_j.x() - p_i.x();
        const double length = ArrayUtils::L2Norm(delta_x);
        return (p_i.m() * p_j.m() / (length * length * length)) * delta_x;
    });
}

void CalculateV(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetV(p.v() + delta_t * (p.f() + p.old_f()) / (2 * p.m()));
    }
}

void RunSimulation(
    ParticleContainer &particles, const struct SimulationSettings settings,
    const std::function<void(const ParticleContainer &, int, double)> &post_iteration) {
    double current_time = settings.startTime;

    int iteration = 0;

    // for this loop, we assume: current x, current f and current v are known
    while (current_time < settings.endTime) {
        CalculateX(particles, settings.deltaT);
        CalculateF(particles);
        CalculateV(particles, settings.deltaT);

        post_iteration(particles, iteration, current_time);

        iteration++;
        current_time += settings.deltaT;
    }
}
}  // namespace simulation
