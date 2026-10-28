#pragma once
#include <memory>

#include "ParticleContainer.h"

namespace output_writer {

enum class OutputFormat {
    XYZ,
#ifdef ENABLE_VTK_OUTPUT
    VTK,
#endif
};

class OutputWriter {
   public:
    virtual ~OutputWriter() = default;
    virtual void Write(const ParticleContainer &particles, int iteration) = 0;

    static std::unique_ptr<OutputWriter> MakeWriter(OutputFormat format, std::string prefix);
};

}  // namespace output_writer
