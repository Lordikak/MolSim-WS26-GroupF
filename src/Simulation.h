#pragma once
#include <cmath>
#include <functional>

#include "LennardJones.h"
#include "ParticleContainer.h"
#include "spdlog/spdlog.h"

namespace simulation {
/**
 * @brief configuration parameters for a simulation run
 *
 * contains positive arguments start time, end time, and delta t
 */
struct SimulationSettings {
    double start_time;
    double end_time;
    double delta_t;
};

namespace detail {
inline void CalculateX(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetX(p.x() + delta_t * p.v() + (delta_t * delta_t / (2 * p.m())) * p.f());
    }
}
inline void CalculateV(ParticleContainer &particles, const double delta_t) {
    for (auto p : particles) {
        p.SetV(p.v() + delta_t * (p.f() + p.old_f()) / (2 * p.m()));
    }
}
}  // namespace detail

/**
 * @brief Molecular dynamic simulation
 */
template <PairKernel Kernel = LennardJones>
class Simulation {
    ParticleContainer particles_;
    SimulationSettings settings_;
    Kernel kernel_;

    /**
     * @brief Computes LJ for each particle pair
     *
     * computed once per pair and then applies to both using newtons third law: F_ij = - F_ji
     *
     * @param particles particles to compute the forces for
     */
    void CalculateF();

   public:
    Simulation(ParticleContainer container, const SimulationSettings &settings, Kernel kernel = {})
        : particles_(std::move(container)), settings_(settings), kernel_(std::move(kernel)) {}

    /**
     * @brief Calculates each iteration of a simulation run
     *
     * calculates forces, positions, and velocities for each particle each iteration
     * beginning at settings.start_time until settings.end_time, with a step size
     * settings.delta_t
     * @param post_iteration callback invoked after each iteration and once before the first one
     *          Arguments-
     *          current state
     *          iteration count
     *          current time
     */
    void Run(const std::function<void(const ParticleContainer &, int, double)> &post_iteration);

    /**
     * @brief The number of iterations with the current `SimulationSettings`
     */
    int IterationCount() const;
};

template <PairKernel Kernel>
void Simulation<Kernel>::CalculateF() {
    // store old force in old_f and reset it, as it is calculated in the next step
    for (auto p : particles_) {
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    particles_.Rebuild();
    particles_.ApplyPairForces(kernel_);
}

template <PairKernel Kernel>
void Simulation<Kernel>::Run(
    const std::function<void(const ParticleContainer &, int, double)> &post_iteration) {
    const int iteration_count = IterationCount();

    SPDLOG_DEBUG("Starting simulation: t = {} .. {}, dt = {}, {} iterations, {} particles",
                 settings_.start_time, settings_.end_time, settings_.delta_t, iteration_count,
                 particles_.size());

    CalculateF();

    post_iteration(particles_, 0, settings_.start_time);

    // for this loop, we assume: current x, current f and current v are known
    for (int iteration = 1; iteration <= iteration_count; ++iteration) {
        detail::CalculateX(particles_, settings_.delta_t);
        CalculateF();
        detail::CalculateV(particles_, settings_.delta_t);

        const double cur_time = settings_.start_time + settings_.delta_t * iteration;
        SPDLOG_TRACE("Iteration {} completed; t = {}", iteration, cur_time);

        post_iteration(particles_, iteration, cur_time);
    }
    SPDLOG_DEBUG("Simulation done");
}

template <PairKernel Kernel>
int Simulation<Kernel>::IterationCount() const {
    return static_cast<int>(
        std::llround((settings_.end_time - settings_.start_time) / settings_.delta_t));
}

}  // namespace simulation
