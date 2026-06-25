#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"

/// @brief Context and Cartesian state for a single leg through the pipeline
struct LegCartesianState
{
    /// @brief Target position of the foot (in Body frame)
    Vec3f target_pos;

    /// @brief Current phase of the step cycle (0.0 to 1.0). Useful for animations/maths.
    float phase = 0.0f;

    /// @brief TRUE if the Gait Planner wants the leg in the air. 
    /// Can be overridden by Terrain Adaptation if an obstacle is hit.
    bool is_swinging = false;

    /// @brief TRUE if the physical foot switch is pressed. (Read from hardware at pipeline start)
    bool is_grounded = false;
};

/// @brief Stores the full cartesian intent and context (Pipeline Input & Buffer)
struct BodyCartesianState
{
    /// @brief Body position offset (used for breathing, crouching, etc.)
    Vec3f body_pos;
    
    /// @brief Body rotation (Roll, Pitch, Yaw). Modifiable by IMU for stabilization.
    Vec3f body_rot;

    /// @brief State and target of each of the 4 legs
    LegCartesianState legs[4];
};

class CartesianStateModifier
{
    virtual Status update(float dt, BodyCartesianState& state) = 0;
};

/// @brief Stores all the rotation of each motors of a leg
struct LegJointState
{
    /// @brief Joint angles (Hip Roll, Hip Pitch, Knee Pitch order)
    float joint_angles_rad[3];
};

/// @brief Stores all the rotation of each body joint
struct BodyJointState
{
    /// @brief Leg joints (FL, BL, BR, FR order)
    LegJointState leg_joints[4];

    /// @brief Left ear joint for animations
    float ear_l_rad = 0.0f;
    /// @brief Right ear joint for animations
    float ear_r_rad = 0.0f;
};

class JointStateModifier
{
    virtual Status update(float dt, BodyJointState& state) = 0;
};
