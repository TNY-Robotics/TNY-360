#include "locomotion/LegKinematics.hpp"
#include "common/Log.hpp"

constexpr const char* TAG = "IK";

Error computeIK(const Vec3f& target, const LegGeometry& geo, LegAngles& out)
{
    // Calculate distances and basic checks
    float dist_zy_raw = sqrt(target.y * target.y + target.z * target.z); // theorical distance
    float dist_zy = sqrt(dist_zy_raw * dist_zy_raw - geo.hip_offset * geo.hip_offset); // adjusted for hip offset
    if (dist_zy > (geo.length_thigh + geo.length_calf))
    {
        Log::Add(Log::Level::Debug, TAG, "Target unreachable: dist_zy %.2f > max %.2f", dist_zy, geo.length_thigh + geo.length_calf);
        return Error::Unreachable; // Target is out of reach
    }

    // Calculate roll angle compensation due to hip offset
    float roll_compensation = atan2(geo.hip_offset, dist_zy);
    // Calculate base hip roll angle
    float hip_roll_base = atan2(-target.y, -target.z);

    // Calculate complete leg distance (including X) -- used for knee/hip pitch calculations
    float dist_leg = sqrt(dist_zy * dist_zy + target.x * target.x);
    if (dist_leg > (geo.length_thigh + geo.length_calf))
    {
        Log::Add(Log::Level::Debug, TAG, "Target unreachable: dist_leg %.2f > max %.2f", dist_leg, geo.length_thigh + geo.length_calf);
        return Error::Unreachable; // Target is out of reach
    }

    // Calculate hip pitch base angle (like if the leg was straight)
    float hip_pitch_base = atan2(target.x, dist_zy);
    // Calculate the hip angle using the law of cosines (al-kashi's theorem)
    float hip_pitch_angle_val = (geo.length_thigh * geo.length_thigh + dist_leg * dist_leg - geo.length_calf * geo.length_calf) /
                                (2.0f * geo.length_thigh * dist_leg);
    float hip_pitch_angle = acos(hip_pitch_angle_val);

    // Calculate the knee angle using the law of cosines too
    float knee_angle_val = (geo.length_thigh * geo.length_thigh + geo.length_calf * geo.length_calf - dist_leg * dist_leg) /
                           (2.0f * geo.length_thigh * geo.length_calf);
    float knee_angle = acos(knee_angle_val);

    // Set output angles
    out.hip_roll = hip_roll_base - roll_compensation;
    out.hip_pitch = hip_pitch_base - hip_pitch_angle;
    out.knee_pitch = PI - knee_angle; // knee is a 0 when fully extended, so we invert the angle
    return Error::None;
}

Error computeFK(const LegAngles& angles, const LegGeometry& geo, Vec3f& out_pos)
{
    // calculate knee position in (Z, X) plane
    float knee_x = geo.length_thigh * sin(angles.hip_pitch);
    float knee_z = -geo.length_thigh * cos(angles.hip_pitch);

    // calculate foot position in (Z, X) plane
    float knee_angle_corrected = angles.hip_pitch + angles.knee_pitch;
    float foot_x = knee_x + geo.length_calf * sin(knee_angle_corrected);
    float foot_z = knee_z - geo.length_calf * cos(knee_angle_corrected);
    // adjust for hip offset in Y direction
    float foot_y = -geo.hip_offset;
    
    // rotate foot position around X axis by hip roll angle (+ assign to out_pos)
    out_pos.x = foot_x;
    out_pos.y = foot_y * cos(angles.hip_roll) + foot_z * sin(angles.hip_roll);
    out_pos.z = - foot_y * sin(angles.hip_roll) + foot_z * cos(angles.hip_roll);

    return Error::None;
}