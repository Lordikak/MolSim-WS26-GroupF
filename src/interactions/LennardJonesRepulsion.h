#pragma once

#include <limits>

#include "../utils/ArrayUtils.h"
#include "../utils/Vec3.h"

/**
 * @brief Lennard-Jones repulsion kernel for computing the force between a particle
 *        and its ghost image at a boundary.
 */
struct LennardJonesRepulsion {
    double epsilon = 5.0;  ///< Depth of the potential well (default from worksheet)
    double sigma = 1.0;    ///< Distance where the potential is 0
    double r_cutoff = std::numeric_limits<double>::infinity();

    /**
     * @brief Computes the repulsive force between a particle and its ghost image.
     *
     * @param position Position of the particle receiving the force
     * @param ghost_x Position of the ghost particle
     * @return Force vector acting on the particle
     */
    Vec3 operator()(Vec3 position, Vec3 ghost_x) const {
        const Vec3 delta_x = ghost_x - position;
        const double r2 = ArrayUtils::L2NormSquared(delta_x);
        const double inv_r2 = 1.0 / r2;
        const double s2 = sigma * sigma * inv_r2;
        const double s6 = s2 * s2 * s2;
        const double s12 = s6 * s6;
        const double scale = -48.0 * epsilon * inv_r2 * s12;
        // The condition is here to help the compiler use masks instead of branching.
        // No Branching = GOOD
        return (r2 < r_cutoff * r_cutoff ? scale : 0.0) * delta_x;
    }
};
