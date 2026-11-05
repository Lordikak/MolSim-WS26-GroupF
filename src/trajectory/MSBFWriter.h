#pragma once

#include <fstream>
#include <span>

#include "MSBFormat.h"
#include "outputWriter/OutputWriter.h"
#include "utils/Vec3.h"

namespace output_writer {
class MSBFWriter final : public OutputWriter {
    std::ofstream data_;
    std::ofstream idx_;
    std::uint64_t offset_ = 0;

    void PutU32(std::uint32_t v);
    void PutU64(std::uint64_t v);
    void PutF64(double v);
    void PutBytes(std::span<const uint8_t> bytes);
    void PutVec3(const Vec3 &v);
    void PutHeader(const msb::Header &v);
    void PutFrameHeader(const msb::FrameHeader &v);

    void WriteIdxEntry(const msb::IndexEntry &entry);

   public:
    explicit MSBFWriter(const std::filesystem::path &dir, const std::string &prefix, double start_time,
                       double dt, const std::array<Vec3, 2> &domain);

    /// @copydoc OutputWriter::Write
    void Write(const ParticleContainer &particles, int iteration) override;
};
}