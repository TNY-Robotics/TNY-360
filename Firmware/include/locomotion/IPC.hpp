#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include "locomotion/Leg.hpp"
#include "locomotion/Joint.hpp"

namespace IPC
{
    constexpr const char* TAG = "IPC";
    constexpr int NB_LEGS = (int) Leg::Id::Count;
    constexpr int NB_JOINTS = (int) Joint::Id::Count;
    
    struct ControlIntent {
        /// @brief Timestamp of the intent command
        uint32_t timestamp_ms = 0;

        /// @brief Desired body velocity in the robot's local frame (vx [m/s], vy [m/s], vz [rad/s])
        Vec3f body_vel = Vec3f::Zero();
        /// @brief Desired body orientation in the robot's local frame (roll [rad], pitch [rad], yaw [rad])
        Vec3f body_rot = Vec3f::Zero();
        /// @brief Desired body position in the ground's frame (x [m], y [m], z [m])
        Vec3f body_pos = Vec3f::Zero();
    };

    struct RobotState {
        /// @brief Timestamp of the state measurements
        uint32_t timestamp_ms = 0;

        /// @brief State of each joint in the robot, ordered by Joint::Id
        struct JointState {
            /// @brief Target angle for the joint [rad]
            float target_angle_rad = 0.0f;
            /// @brief Feedback angle from the joint [rad]
            float feedback_angle_rad = 0.0f;
            /// @brief Model angle for the joint [rad]
            float model_angle_rad = 0.0f;
            /// @brief Estimated angle for the joint [rad]
            float estimated_angle_rad = 0.0f;
        } joints[NB_JOINTS];

        /// @brief Orientation of the robot in world frame
        Quatf orientation = Quatf::Identity();
        /// @brief Which legs are currently grounded, ordered by Leg::Id
        bool grounded[NB_LEGS] = {true, true, true, true};
    };

    /// @brief Initialize the IPC system.
    /// @return Error code indicating success or failure of the operation.
    Status Init();

    /// @brief Deinitialize the IPC system.
    /// @return Error code indicating success or failure of the operation.
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
