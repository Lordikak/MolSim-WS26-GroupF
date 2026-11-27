#pragma once

enum class Face : std::uint8_t { kXMin = 0, kXMax, kYMin, kYMax, kZMin, kZMax };
enum class BoundaryCondition : std::uint8_t { kOutflow, kReflecting };
