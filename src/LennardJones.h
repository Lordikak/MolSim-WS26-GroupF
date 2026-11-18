#pragma once

#include <limits>

#include "utils/ArrayUtils.h"
#include "utils/Vec3.h"

/**
 * @brief Lennard Jones force kernel to get the force between two particles
 */
struct LennardJones {
    double epsilon = 5.0;  ///< Depth of the potential well (default from worksheet)
    double sigma = 1.0;    ///< Distance where the potential is 0
    double r_cutoff = std::numeric_limits<double>::infinity();

    /**
     * @brief Computes force vector
     *
     * @param p_i Particle the force acts on
     * @param p_j Particle exerting the force
     * @return Force vector on @p p_i by @p p_j
     */
    Vec3 operator()(auto p_i, auto p_j) const {
        const Vec3 delta_x = p_j.x() - p_i.x();
        const double r2 = ArrayUtils::L2NormSquared(delta_x);
        const double inv_r2 = 1.0 / r2;
        const double s2 = sigma * sigma * inv_r2;
        const double s6 = s2 * s2 * s2;
        const double s12 = s6 * s6;
        const double scale = (24.0 * epsilon * inv_r2 * (s6 - 2.0 * s12));
        // The condition is here to help the compiler use masks instead of branching.
        // No Branching = GOOD
        return (r2 < r_cutoff * r_cutoff ? scale : 0.0) * delta_x;
    }

    /**
     * @brief Compute potential energy
     * @return potential energy between two particles
     */
    double Potential(auto p_i, auto p_j) const {
        const Vec3 delta_x = p_j.x() - p_i.x();
        const double r2 = ArrayUtils::L2NormSquared(delta_x);
        const double inv_r2 = 1.0 / r2;
        const double s2 = sigma * sigma * inv_r2;
        const double s6 = s2 * s2 * s2;
        const double s12 = s6 * s6;
        return 4.0 * epsilon * (s12 - s6);
    }
};
