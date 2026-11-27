#pragma once
#include <yaml-cpp/yaml.h>

#include <string>

#include "FileReader.h"
#include "SimulationConfig.h"

namespace file_reader {
/**
 * @brief Reads particle and domain initialization data from YAML file
 *
 * Input file supplied on construction
 *
 * Format:
 *  - `version`: format version *REQUIRED
 *  - `cuboids`: array of cuboids, each one generating a 3D grid of particles
 *     `origin`: The min min min corner `[x, y, z]`
 *     `dimensions`: number of particles in dimensions `[N1, N2, N3]`, all > 0
 *     `spacing`: distance between neighboring particles > 0
 *     `mass`: mass of each particle > 0
 *     `initial_velocity`: velocity of the whole cuboid `[vx, vy, vz]`
 *     `brownian_velocity`: mean vel of the brownian motion, >= 0
 *
 *  - `discs`: array of cylinders, each one generating a 3D grid of particles
 *     `center`: The center of the lower face `[x, y, z]`
 *     `radius`: number of particles along the radius of the lower face
 *     `height`: The number of particles between the lower and upper face inclusive >= 0
 *     `spacing`: distance between neighboring particles > 0
 *     `mass`: mass of each particle > 0
 *     `initial_velocity`: velocity of the whole cylinder `[vx, vy, vz]`
 *     `brownian_velocity`: mean vel of the brownian motion, >= 0
 *
 *  - 'bounds_minimum' : position of the min min min bound of the domain *REQUIRED
 *  - 'bounds_maximum' : position of the max max max bound of the domain *REQUIRED
 *  - 'cutoff' : value of the cutoff radius of the lennard jones forces *REQUIRED
 *  - 'boundary_conditions' : mapping of six domain faces to their boundary behavior *REQUIRED
 *       min_x : reflecting || outflow
 *       max_x ...
 *       min_y
 *       max_y
 *       min_z
 *       max_z
 *
 *    note : subfields of boundary_condition to be provided in the above order
 *
 *  - start : non-negative double start time of the simulation
 *  - end : non-negative double end time of the simulation >= start
 *  - delta_t : positive double timestep of the simulation < end-start
 *
 * Example:
 * @code
version: 1
cuboids:
  - origin: [20, 20, 0.0]
    dimensions: [20, 20, 1]
    spacing: 1.1225
    mass: 1.0
    initial_velocity: [0.0, 0.0, 0.0]
    brownian_velocity: 0.1

  - origin: [70, 60, 0.0]
    dimensions: [20, 20, 1]
    spacing: 1.1225
    mass: 1.0
    initial_velocity: [0.0, -10, 0.0]
    brownian_velocity: 0.1

bounds_minimum: [0.0,0.0,0.0]
bounds_maximum: [100.0, 100.0, 100.0]
cutoff: 3.0

boundary_conditions:
  min_x: reflecting
  max_x: reflecting
  min_y: reflecting
  max_y: reflecting
  min_z: outflow
  max_z: outflow

start: 1.0
end: 10.0
delta_t: 0.005
 * @endcode
 */

class YamlReader final : public FileReader {
    const std::string file_path_;

    /**
     * @brief initialize the particle generators based on yaml intialization data
     * @param value the cuboid or disc to be defined
     * @return a vector of ParticleCuboids and ParticleDiscs to initialize molecules later
     */
    GeneratorVector InitializeGenerators(const YAML::Node &value);

   public:
    /**
     * @brief Constructs a reader for the specified YAML file.
     * @param file_path Path to the input YAML file.
     */
    explicit YamlReader(std::string file_path) : file_path_(std::move(file_path)) {}

    /**
     * @brief Adds particles read from the YAML file to the container.
     * @param user_defined user defined simulation config settings
     * in the case that the parsed file does not contain them
     * @return SimulationSpec struct containing configuration data for simulation and domain,
     * along with particle generators
     */
    SimulationSpec Read(SimulationSettings user_defined) override;
};
}  // namespace file_reader
