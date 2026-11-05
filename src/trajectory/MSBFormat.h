#pragma once
#include <cstdint>
#include <array>
#include <type_traits>
#include "utils/Vec3.h"

namespace msb {
constexpr std::uint32_t kFmtVersion = 100;
constexpr std::array<std::uint8_t, 4> kMagic = {'m', 's', 'b', 'f'};

struct Header {
    std::array<std::uint8_t, 4> magic = kMagic;
    std::uint32_t version = kFmtVersion;
    std::uint64_t fields = 0;
    double start_time = 0;
    double dt = 0;
    std::array<Vec3, 2> domain{};
};
static_assert(sizeof(Header) == 80);
static_assert(std::is_trivially_copyable_v<Header>);

struct FrameHeader {
    std::uint32_t step = 0;
    std::uint32_t n = 0;
};

struct IndexEntry {
    std::uint64_t offset;
    std::uint32_t step;
    std::uint32_t n;
};
static_assert(sizeof(IndexEntry) == 16);
}
