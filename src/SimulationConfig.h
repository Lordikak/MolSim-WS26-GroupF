#pragma once

#include "ParticleGenerator.h"
#include "utils/BoundaryFace.h"
#include "utils/Vec3.h"

/**
 * @brief configuration settings related to the form of the domain
 *
 * contains the lower left and upper right most positions in the domain
 * the cutoff value for the simulation, and the boundary condition of each face
 */
struct DomainSettings {
    Vec3 min{0, 0, 0};
    Vec3 max{100, 100, 100};
    double cutoff{3.0};
    std::array<BoundaryCondition, 6> boundary_conditions{BoundaryCondition::kOutflow};
};

/**
 * @brief configuration settings related to how the simulation runs
 *
 * defines start time, end time, and time step
 */
struct SimulationSettings {
    double start_time;
    double end_time;
    double delta_t;
};

/**
 * @brief configuration settings for simulation and domain and generators for particle shapes
 */
struct SimulationSpec {
    SimulationSettings settings;
    DomainSettings domain;
    GeneratorVector generators;
};
