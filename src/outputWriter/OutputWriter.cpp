#include "OutputWriter.h"

#include "VTKWriter.h"
#include "XYZWriter.h"

namespace output_writer {

std::unique_ptr<OutputWriter> OutputWriter::MakeWriter(OutputFormat format, std::string prefix) {
    switch (format) {
        case OutputFormat::XYZ:
            return std::make_unique<XYZWriter>(prefix);
#ifdef ENABLE_VTK_OUTPUT
        case OutputFormat::VTK:
            return std::make_unique<VTKWriter>(std::move(prefix));
#endif
    }
    std::abort();  // unreachable
}

}  // namespace output_writer