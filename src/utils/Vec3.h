#pragma once
#include <array>
#include <cstddef>

#include "utils/ArrayUtils.h"
#include "utils/ArrayVector.h"

constexpr std::size_t DIM = 3;

using Vec3 = std::array<double, DIM>;
using Vec3Vector = ArrayVector<double, DIM>;