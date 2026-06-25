#include "locomotion/KinematicsEngine.hpp"
#include <cmath>
#include "common/Log.hpp"

KinematicsEngine::KinematicsEngine()
{
}

KinematicsEngine::KinematicsEngine(KinematicsConfig config)
    : config(config)
{
}

Status KinematicsEngine::computeBodyIK(const BodyCartesianState& cartesian, BodyJointState& joints)
{
    Transformf body_transform(cartesian.body_pos, Quatf::FromEulerAngles(cartesian.body_rot));

    Vec3f hip_positions[4] = {
        Vec3f( config.hip_shift_x,  config.hip_shift_y, 0.f), // FL
        Vec3f(-config.hip_shift_x,  config.hip_shift_y, 0.f), // BL
        Vec3f(-config.hip_shift_x, -config.hip_shift_y, 0.f), // BR
        Vec3f( config.hip_shift_x, -config.hip_shift_y, 0.f), // FR
    };

    for (int i = 0; i < 4; i++)
    {
        // Feet position in world frame (from gait planner)
        const Vec3f& foot_target = cartesian.legs[i].target_pos;
        // Feet position from world frame to body frame
        Vec3f foot_in_body_frame = body_transform.worldToLocal(foot_target);
        // Feet position from body frame to hip frame
        Vec3f target_hip_frame = foot_in_body_frame - hip_positions[i];

        // Invert target y position if leg is inverted
        if (config.leg_inverted[i])
        {
            target_hip_frame.y = -target_hip_frame.y;
        }

        // Calculate leg IK
        if (Status err = computeLegIK(target_hip_frame, joints.leg_joints[i]); err != Status::Ok)
        {
            LOG_ERROR(TAG, "IK Failed for leg %d", i);
            return err;
        }
    }

    return Status::Ok;
}

Status KinematicsEngine::computeLegIK(const Vec3f& target, LegJointState& joints)
{
    float dist_zy_sq = target.y * target.y + target.z * target.z;
    float dist_zy = sqrtf(dist_zy_sq - config.hip_offset * config.hip_offset);

    if (dist_zy > (config.length_thigh + config.length_calf)) {
        LOG_ERROR(TAG, "Feet position is too far (%.2fm)", dist_zy);
        return Status::OutOfBounds; 
    }

    float roll_compensation = atan2f(config.hip_offset, dist_zy);
    float hip_roll_base = atan2f(-target.y, -target.z);

    float dist_leg = sqrtf(dist_zy * dist_zy + target.x * target.x);
    if (dist_leg > (config.length_thigh + config.length_calf)) {
        LOG_ERROR(TAG, "Feet position is too far after hip roll compensation (%.2fm)", dist_leg);
        return Status::OutOfBounds;
    }

    float hip_pitch_base = atan2f(target.x, dist_zy);
    
    float thigh_sq = config.length_thigh * config.length_thigh;
    float calf_sq = config.length_calf * config.length_calf;
    float dist_leg_sq = dist_leg * dist_leg;

    float hip_pitch_angle_val = (thigh_sq + dist_leg_sq - calf_sq) / (2.0f * config.length_thigh * dist_leg);
    float hip_pitch_angle = acosf(hip_pitch_angle_val);

    float knee_angle_val = (thigh_sq + calf_sq - dist_leg_sq) / (2.0f * config.length_thigh * config.length_calf);
    float knee_angle = acosf(knee_angle_val);

    joints.joint_angles_rad[0] = hip_roll_base - roll_compensation;
    joints.joint_angles_rad[1] = hip_pitch_base - hip_pitch_angle;
    joints.joint_angles_rad[2] = PI - knee_angle; 
    
    return Status::Ok;
}
