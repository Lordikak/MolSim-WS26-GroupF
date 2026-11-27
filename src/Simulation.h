#pragma once
#include <cmath>
#include <functional>

#include "ParticleContainer.h"
#include "interactions/LennardJones.h"
#include "interactions/LennardJonesRepulsion.h"
#include "spdlog/spdlog.h"

namespace simulation {
/**
 * @brief configuration parameters for a simulation run
 *
 * contains positive arguments start time, end time, delta t and gravity
 */
struct SimulationSettings {
    double start_time;
    double end_time;
    double delta_t;

    Vec3 gravity{0, 0, 0};
};

namespace detail {
inline void CalculateX(ParticleContainer &particles, const double delta_t) {
    const auto n = static_cast<std::int64_t>(particles.size());
#pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < n; ++i) {
        ParticleRef p(particles, i);
        p.SetX(p.x() + delta_t * p.v() + (delta_t * delta_t / (2 * p.m())) * p.f());
    }
}
inline void CalculateV(ParticleContainer &particles, const double delta_t) {
    const auto n = static_cast<std::int64_t>(particles.size());
#pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < n; ++i) {
        ParticleRef p(particles, i);
        p.SetV(p.v() + delta_t * (p.f() + p.old_f()) / (2 * p.m()));
    }
}
}  // namespace detail

/**
 * @brief Molecular dynamic simulation
 */
template <PairKernel Kernel = LennardJones, class BoundaryKernel = LennardJonesRepulsion>
class Simulation {
    ParticleContainer particles_;
    SimulationSettings settings_;
    Kernel kernel_;
    BoundaryKernel boundary_kernel_;

    /**
     * @brief Computes LJ for each particle pair
     *
     * computed once per pair and then applies to both using newtons third law: F_ij = - F_ji
     */
    void CalculateF();

   public:
    Simulation(ParticleContainer container, const SimulationSettings &settings, Kernel kernel = {},
               BoundaryKernel boundary_kernel = {})
        : particles_(std::move(container)),
          settings_(settings),
          kernel_(std::move(kernel)),
          boundary_kernel_(std::move(boundary_kernel)) {}

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

template <PairKernel Kernel, class BoundaryKernel>
void Simulation<Kernel, BoundaryKernel>::CalculateF() {
    particles_.Rebuild();
    const auto n = static_cast<std::int64_t>(particles_.size());
#pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < n; ++i) {
        ParticleRef p(particles_, i);
        p.SetOldF(p.f());
        p.SetF(Vec3{});
    }

    particles_.ApplyPairForces(kernel_);
    particles_.ApplyBoundryForces(boundary_kernel_);

    // TODO think how better control external forces?
    if (settings_.gravity[0] == 0.0 && settings_.gravity[1] == 0.0 && settings_.gravity[2] == 0.0) {
        return;
    }
    particles_.ApplyExternalForces(settings_.gravity);
}

template <PairKernel Kernel, class BoundaryKernel>
void Simulation<Kernel, BoundaryKernel>::Run(
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

template <PairKernel Kernel, class BoundaryKernel>
int Simulation<Kernel, BoundaryKernel>::IterationCount() const {
    return static_cast<int>(
        std::llround((settings_.end_time - settings_.start_time) / settings_.delta_t));
}

}  // namespace simulation
