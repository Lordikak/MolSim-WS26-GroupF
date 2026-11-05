#include "MSBFWriter.h"

#include <bit>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <array>
#include <limits>

#include "MSBFormat.h"
#include "ParticleContainer.h"

namespace output_writer {
static auto PrepareDir(const std::filesystem::path &dir) {
    std::filesystem::create_directories(dir);
    return dir;
}

void MSBFWriter::PutBytes(const std::span<const std::uint8_t> bytes) {
    data_.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    offset_ += bytes.size();
}
void MSBFWriter::PutU32(const std::uint32_t v) { PutBytes(std::bit_cast<std::array<std::uint8_t, 4>>(v)); }
void MSBFWriter::PutU64(const std::uint64_t v) { PutBytes(std::bit_cast<std::array<std::uint8_t, 8>>(v)); }
void MSBFWriter::PutF64(const double v)        { PutBytes(std::bit_cast<std::array<std::uint8_t, 8>>(v)); }
void MSBFWriter::PutVec3(const Vec3 &v)        { for (const double d : v) PutF64(d); }
void MSBFWriter::PutHeader(const msb::Header &v) { PutBytes(std::bit_cast<std::array<std::uint8_t, sizeof(msb::Header)>>(v)); }
void MSBFWriter::PutFrameHeader(const msb::FrameHeader &v) { PutBytes(std::bit_cast<std::array<std::uint8_t, sizeof(msb::FrameHeader)>>(v)); }

MSBFWriter::MSBFWriter(const std::filesystem::path &dir, const std::string &prefix, const double start_time,
                       const double dt, const std::array<Vec3, 2> &domain) : data_(PrepareDir(dir) / (prefix+".msb"), std::ios::binary | std::ios::trunc), idx_(dir / (prefix + ".msb.idx"), std::ios::binary | std::ios::trunc) {
    if (!data_ || !idx_) throw std::runtime_error("MSB: cannot open output in " + dir.string());

    data_.exceptions(std::ios::failbit | std::ios::badbit);
    idx_.exceptions(std::ios::failbit | std::ios::badbit);

    PutHeader({.start_time = start_time, .dt = dt, .domain = domain});

    data_.flush();
}

void MSBFWriter::WriteIdxEntry(const msb::IndexEntry &entry) {
    const auto bytes = std::bit_cast<std::array<std::uint8_t, sizeof(msb::IndexEntry)>>(entry);
    idx_.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    idx_.flush();
}

void MSBFWriter::Write(const ParticleContainer &particles, int iteration) {
    assert(iteration >= 0);
    assert(particles.size() <= std::numeric_limits<std::uint32_t>::max());
    const std::uint64_t frame_start = offset_;

    const auto step = static_cast<std::uint32_t>(iteration);
    const auto n = static_cast<std::uint32_t>(particles.size());

    PutFrameHeader({step, n});

    for (auto p : particles) {
        PutVec3(p.x());
    }
    for (auto p : particles) {
        PutVec3(p.v());
    }
    data_.flush();

    WriteIdxEntry({frame_start, step, n});
}
}
