#include "OutputWriter.h"

#include "VTKWriter.h"
#include "XYZWriter.h"
#include "trajectory/MSBFWriter.h"

namespace output_writer {

std::unique_ptr<OutputWriter> OutputWriter::MakeWriter(OutputFormat format,
                                                       const std::filesystem::path &dir,
                                                       std::string prefix, double start_time, double dt) {
    switch (format) {
        case OutputFormat::kXyz:
            return std::make_unique<XYZWriter>(dir / "XYZ", std::move(prefix));
        case OutputFormat::kMsb:
            return std::make_unique<MSBFWriter>(dir / "MSB", std::move(prefix), start_time, dt, std::array<Vec3, 2>{});
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