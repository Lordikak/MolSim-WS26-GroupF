#pragma once
#include <memory>

#include "ParticleContainer.h"

namespace output_writer {

/**
 * @brief Formats for outputs the simulation can export
 */
enum class OutputFormat {
    XYZ,  ///< Plain text XYZ file, only contains Positions
#ifdef ENABLE_VTK_OUTPUT
    VTK,  ///< VTK unstructured file (.vtu)
#endif
};

/**
 * @brief Interface for writing Particles
 */
class OutputWriter {
   public:
    virtual ~OutputWriter() = default;

    /**
     * @brief Writes the Particles to the file
     * @param particles particles to write
     * @param iteration Iteration to write, used in the file name
     */
    virtual void Write(const ParticleContainer &particles, int iteration) = 0;

    /**
     * @brief Factory for a Writer in the specified @p format
     * @param format File format
     * @param prefix File name prefix, files get named: `<prefix>_<iteration>.<extension>`
     * @return Writer in the @p format
     */
    static std::unique_ptr<OutputWriter> MakeWriter(OutputFormat format, std::string prefix);
};

}  // namespace output_writer
