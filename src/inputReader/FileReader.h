#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "ParticleContainer.h"
#include "SimulationConfig.h"

namespace file_reader {

/**
 * @brief Interface for reading input files
 */
class FileReader {
   public:
    virtual ~FileReader() = default;

    /**
     * @brief Reads particles from the input file into the container.
     * @param particles Container to which the particles are added.
     * @param user_defined user defined simulation config settings in the case
     * that the parsed file does not contain them
     * @return SimulationSpec struct containing configuration data for simulation and domain,
     * along with particle generators
     */
    virtual SimulationSpec Read(SimulationSettings user_defined) = 0;

    /**
     * @brief Factory for a Reader for the filetype (specified in the file_path)
     *
     * @param file_path path to input file
     * @return Reader for the filetype (specified in the @p file_path)
     */
    static std::unique_ptr<FileReader> MakeReader(const std::string &file_path);
};

}  // namespace file_reader
