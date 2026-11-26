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
 *
 * Example:
 * @code
 * @endcode
 */

class YamlReader final : public FileReader {
    const std::string file_path_;

    /**
     * @brief initialize the particle generators based on yaml intialization data
     * @param value the cuboid or disc to be defined
     * @return a vector of ParticleCuboids and ParticleDiscs to initialize molecules later
     */
    static GeneratorVector InitializeGenerators(const YAML::Node &value);

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
