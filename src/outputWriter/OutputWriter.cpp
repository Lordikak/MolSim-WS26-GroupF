#include "OutputWriter.h"

#include "VTKWriter.h"
#include "XYZWriter.h"

namespace output_writer {

std::unique_ptr<OutputWriter> OutputWriter::MakeWriter(OutputFormat format,
                                                       const std::filesystem::path &dir,
                                                       std::string prefix) {
    switch (format) {
        case OutputFormat::kXyz:
            return std::make_unique<XYZWriter>(dir / "XYZ", std::move(prefix));
#ifdef ENABLE_VTK_OUTPUT
        case OutputFormat::kVtk:
            return std::make_unique<VTKWriter>(dir / "VTK", std::move(prefix), false);
        case OutputFormat::kVtkAscii:
            return std::make_unique<VTKWriter>(dir / "VTK_ASCII", std::move(prefix), true);
#endif
    }
    std::abort();  // unreachable
}

}  // namespace output_writer