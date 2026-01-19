#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include <cmath>

typedef struct LegAngles {
    float hip_roll;   // Hip roll angle in radians
    float hip_pitch;  // Hip pitch angle in radians
    float knee_pitch; // Knee pitch angle in radians
} LegAngles;

typedef struct LegGeometry {
    float hip_offset;
    float length_thigh;
    float length_calf;
} LegGeometry;

Error computeIK(const Vec3f& target, const LegGeometry& geo, LegAngles& out);

Error computeFK(const LegAngles& angles, const LegGeometry& geo, Vec3f& out_pos);
