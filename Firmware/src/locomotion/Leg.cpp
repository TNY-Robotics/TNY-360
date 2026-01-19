#include "locomotion/Leg.hpp"
#include "common/Log.hpp"

Leg::Leg(Joint hip_roll, Joint hip_pitch, Joint knee_pitch, bool y_inverted)
    : hip_roll(hip_roll), hip_pitch(hip_pitch), knee_pitch(knee_pitch), y_inverted(y_inverted)
{
}

Error Leg::init()
{
    Error err;

    if ((err = hip_roll.init()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.init()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.init()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Leg::deinit()
{
    Error err;

    if ((err = hip_roll.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.deinit()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Leg::update()
{
    Error err;

    if ((err = hip_roll.update()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.update()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.update()) != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Leg::enable()
{
    Error err;

    if ((err = hip_roll.enable()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.enable()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.enable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Leg::disable()
{
    Error err;

    if ((err = hip_roll.disable()) != Error::None)
    {
        return err;
    }
    if ((err = hip_pitch.disable()) != Error::None)
    {
        return err;
    }
    if ((err = knee_pitch.disable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Leg::setTarget(const Vec3f& target_pos)
{
    // calculate IK angles
    LegAngles angles;
    if (Error err = calculate_ik_angles(target_pos, angles); err != Error::None)
    {
        return err;
    }

    float time_estimate_s = getTimeEstimate(target_pos);

    // set the joint targets with the same time to reach
    if (Error err = hip_roll.setTarget_Timed(angles.hip_roll, time_estimate_s); err != Error::None)
    {
        return err;
    }
    if (Error err = hip_pitch.setTarget_Timed(angles.hip_pitch, time_estimate_s); err != Error::None)
    {
        return err;
    }
    if (Error err = knee_pitch.setTarget_Timed(angles.knee_pitch, time_estimate_s); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Leg::setTarget_Timed(const Vec3f& target_pos, float time_s)
{
    // calculate IK angles
    LegAngles angles;
    if (Error err = calculate_ik_angles(target_pos, angles); err != Error::None)
    {
        return err;
    }

    // set the joint targets with the same time to reach
    if (Error err = hip_roll.setTarget_Timed(angles.hip_roll, time_s); err != Error::None)
    {
        return err;
    }
    if (Error err = hip_pitch.setTarget_Timed(angles.hip_pitch, time_s); err != Error::None)
    {
        return err;
    }
    if (Error err = knee_pitch.setTarget_Timed(angles.knee_pitch, time_s); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

float Leg::getTimeEstimate(const Vec3f& target_pos) const
{
    LegAngles angles;
    if (Error err = calculate_ik_angles(target_pos, angles); err != Error::None)
    {
        return -1.f;
    }

    float time_hip_roll = hip_roll.getTimeEstimate(angles.hip_roll);
    float time_hip_pitch = hip_pitch.getTimeEstimate(angles.hip_pitch);
    float time_knee_pitch = knee_pitch.getTimeEstimate(angles.knee_pitch);

    if (time_hip_roll < 0.f || time_hip_pitch < 0.f || time_knee_pitch < 0.f)
    {
        return -1.f;
    }

    return std::max(time_hip_roll, std::max(time_hip_pitch, time_knee_pitch));
}

Error Leg::calculate_ik_angles(const Vec3f& target_pos, LegAngles& angles) const
{
    LegGeometry geo = { // mm
        .hip_offset = HIP_OFFSET_MM,
        .length_thigh = LEG_THIGH_LENGTH_MM,
        .length_calf = LEG_CALF_LENGTH_MM
    };
    
    Vec3f adjusted_target = target_pos;
    if (y_inverted)
    {
        adjusted_target.y = -adjusted_target.y;
    }

    return computeIK(adjusted_target, geo, angles);
}