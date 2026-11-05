#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <vector>
#include <limits>

#include "MSBFormat.h"
#include "utils/Vec3.h"

namespace msb {

struct Frame {
    static constexpr std::uint32_t kNone = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t step = kNone;
    std::uint32_t n = 0;
    std::vector<Vec3> positions;
    std::vector<Vec3> vels;
};

class MSBFReader {
    std::filesystem::path data_path_;
    std::filesystem::path idx_path_;
    std::ifstream data_;
    std::ifstream idx_;
    Header header_;
    std::vector<IndexEntry> entries_;

    Frame frame_;

    static void ReadBytes(std::ifstream &in, std::span<std::byte> out);
    Header ReadHeader();
    IndexEntry ReadIndexEntry();
    FrameHeader ReadFrameHeader();

    std::uint64_t FrameSize(std::uint32_t n) const { //TODO property mask
        return sizeof(FrameHeader) + n * 2 * sizeof(Vec3);
    }

    void LoadIndex(std::size_t i);

public:
    explicit MSBFReader(const std::filesystem::path &msb);
    std::size_t Refresh();
    const Frame &LoadAtStep(std::uint32_t step);
    const Frame &LoadAtTime(double time);
    std::uint32_t FirstStep() const;
    std::uint32_t LastStep() const;
    const Header &header() const;
};
}