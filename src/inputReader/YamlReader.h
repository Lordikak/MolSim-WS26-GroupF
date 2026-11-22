#pragma once
#include <yaml-cpp/yaml.h>

#include <string>

#include "FileReader.h"

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

   public:
    /**
     * @brief Constructs a reader for the specified YAML file.
     * @param file_path Path to the input YAML file.
     */
    explicit YamlReader(std::string file_path) : file_path_(std::move(file_path)) {}

    /**
     * @brief Adds particles read from the YAML file to the container.
     * @param particles Container to which the particles are added.
     */
    void Read(ParticleContainer &particles) override;
};
}  // namespace file_reader
