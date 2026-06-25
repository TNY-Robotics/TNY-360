#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include "locomotion/GaitPlanner.hpp"
#include "locomotion/Leg.hpp"
#include "locomotion/Joint.hpp"

namespace IPC
{
    constexpr const char* TAG = "IPC";
    constexpr int NB_LEGS = (int) Leg::Id::Count;
    constexpr int NB_JOINTS = (int) Joint::Id::Count;
    
    enum class OverrideMode {
        None,      // No override
        Absolute,  // Replace the IK result with the given angle
        Relative,  // Add the given angle to the IK result
    };
    
    struct JointOverride {
        OverrideMode mode = OverrideMode::None;
        float value_rad = 0.0f;
    };

    struct LegOverride {
        OverrideMode mode = OverrideMode::None;
        Vec3f value_pos = Vec3f::Zero();
    };

    struct BodyOverride {
        OverrideMode mode = OverrideMode::None;
        Vec3f value_pos = Vec3f::Zero();
        Vec3f value_rot = Vec3f::Zero();
    };
    
    struct ControlIntent {
        uint32_t timestamp_ms = 0; // timestamp for safeguard watchdog

        // movement planning infos
        GaitPlanner::GaitType gait = GaitPlanner::GaitType::Walk;
        Vec3f body_vel = Vec3f::Zero();
        Vec3f body_rot = Vec3f::Zero();
        Vec3f body_pos = Vec3f::Zero();

        // cartesian override for animation
        LegOverride leg_overrides[NB_LEGS];
        BodyOverride body_override;
    
        // joint control override
        JointOverride joint_overrides[NB_JOINTS];
    };

    struct RobotState {
        uint32_t timestamp_ms = 0;

        struct JointState {
            float target_angle_rad = 0.0f;
            float feedback_angle_rad = 0.0f;
            float model_angle_rad = 0.0f;
            float estimated_angle_rad = 0.0f;
        } joints[NB_JOINTS];

        Vec3f body_orientation = Vec3f::Zero();
        Vec3f imu_down_vector = Vec3f::Zero();
    };

    Status Init();

    Status Deinit();

    /// @brief [Brain Core] Set the control intent to be read by the Reflex core.
    /// @param intent The control intent to set.
    /// @return Error code indicating success or failure of the operation.
    Status setIntent(ControlIntent& intent);

    /// @brief [Reflex Core] Get the latest intent from the Brain core. 
    /// @param intent Pointer to a ControlIntent struct to be filled with the latest intent.
    /// @return True if an intent was received, false otherwise.
    bool getIntent(ControlIntent* intent);

    /// @brief [Reflex Core] Set the robot state to be read by the Brain core.
    /// @param state The robot state to set.
    /// @return Error code indicating success or failure of the operation.
    Status setState(RobotState& state);

    /// @brief [Brain Core] Get the latest robot state from the Reflex core.
    /// @param state Pointer to a RobotState struct to be filled with the latest state.
    /// @return True if a state was received, false otherwise.
    bool getState(RobotState* state);
};
