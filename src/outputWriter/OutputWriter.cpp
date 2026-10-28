#include "OutputWriter.h"

#include "VTKWriter.h"
#include "XYZWriter.h"

namespace output_writer {

std::unique_ptr<OutputWriter> OutputWriter::MakeWriter(OutputFormat format, std::string prefix) {
    switch (format) {
        case OutputFormat::kXyz:
            return std::make_unique<XYZWriter>(std::move(prefix));
#ifdef ENABLE_VTK_OUTPUT
        case OutputFormat::kVtk:
            return std::make_unique<VTKWriter>(std::move(prefix), false);
        case OutputFormat::kVtkAscii:
            return std::make_unique<VTKWriter>(std::move(prefix), true);
#endif
    }
    std::abort();  // unreachable
}

}  // namespace output_writer