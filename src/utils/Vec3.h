#pragma once
#include <array>
#include <cstddef>

#include "utils/ArrayVector.h"
#include "utils/ArrayUtils.h"

constexpr std::size_t DIM = 3;

using Vec3 = std::array<double, DIM>;
using Vec3Vector = ArrayVector<double, DIM>;