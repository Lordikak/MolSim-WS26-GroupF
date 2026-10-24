#pragma once
#include <array>
#include <cstddef>

#include "utils/ArrayUtils.h"
#include "utils/ArrayVector.h"

constexpr std::size_t kDim = 3;

using Vec3 = std::array<double, kDim>;
using Vec3Vector = ArrayVector<double, kDim>;