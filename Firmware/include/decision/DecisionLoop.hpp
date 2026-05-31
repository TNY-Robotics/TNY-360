#pragma once
#include <freertos/FreeRTOS.h>
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include "locomotion/IPC.hpp"

namespace AutoLifeFlags {
    // No auto life features enabled
    constexpr uint8_t None = 0;
    // Use safeguards (avoid fall, self-righting, wall collision, etc.)
    constexpr uint8_t Safeguard = 1 << 0;
    // Automatically choose gait based on conditions (velocity, terrain, etc.)
    constexpr uint8_t AutoGait = 1 << 1;
    // Automatically adjust body posture based on conditions (velocity, terrain, etc.)
    constexpr uint8_t AutoPosture = 1 << 2;
    // Enable animations (idle, walk, run, etc.)
    constexpr uint8_t Animate = 1 << 3;
    // Enable wandering behavior when no command is given
    constexpr uint8_t Wandering = 1 << 4;
}

namespace AutoLifeLevel
{
    // All auto life features disabled (safeguard, auto gait, auto posture, animations, wandering)
    constexpr uint8_t Off = AutoLifeFlags::None;
    // Only safeguards enabled (avoid fall, self-righting, wall collision, etc.)
    constexpr uint8_t Safeguard = AutoLifeFlags::Safeguard;
    // Safeguard, animations, and automatic gait+posture selection based on conditions (velocity, terrain, etc.)
    constexpr uint8_t Animate = AutoLifeFlags::Safeguard | AutoLifeFlags::AutoGait | AutoLifeFlags::AutoPosture | AutoLifeFlags::Animate;
    // Full auto life mode: Safeguard, animations, automatic gait+posture selection, and wandering behavior enabled
    constexpr uint8_t Full = AutoLifeFlags::Safeguard | AutoLifeFlags::AutoGait | AutoLifeFlags::AutoPosture | AutoLifeFlags::Animate | AutoLifeFlags::Wandering;
}

class DecisionLoop
{
public:
    constexpr static const char* TAG = "DecisionLoop";

    /**
     * @brief Initializes the Decision Loop module
     */
    Error init();

    /**
     * @brief Deinitializes the Decision Loop module
     */
    Error deinit();

    /**
     * @brief Starts the decision loop
     * @note This starts a new freertos task.
     */
    Error start();

    /**
     * @brief Stops the decision loop
     */
    Error stop();

    /**
     * @brief Sets the auto life level (See AutoLifeLevel and AutoLifeFlags for available features and levels)
     * @param level The auto life level to set
     * @return Error code indicating success or failure of the operation
     */
    Error setAutoLifeLevel(uint8_t level);

    uint8_t getAutoLifeLevel() const { return auto_life_level; }

    const IPC::ControlIntent& getControlIntent() const { return intent; }

    const IPC::RobotState& getRobotState() const { return state; }

    /// External movement requests

    /**
     * @brief Request a body velocity (robot movement).
     * @param x_ms Linear velocity in the x direction (m/s)
     * @param y_ms Linear velocity in the y direction (m/s)
     * @param z_rads Angular velocity around the vertical axis (rad/s)
     * @return Error code indicating success or failure of the operation.
     * @note This velocity will be applied or not depending on the Decision Loop's internal logic (safeguard, auto life level, etc.)
     */
    Error askBodyVelocity(float x_ms, float y_ms, float z_rads);

    /**
     * @brief Request a body rotation (robot orientation).
     * @param x_rad Rotation around the x axis (roll) in radians
     * @param y_rad Rotation around the y axis (pitch) in radians
     * @param z_rad Rotation around the z axis (yaw) in radians
     * @return Error code indicating success or failure of the operation.
     * @note This rotation will be applied or not depending on the Decision Loop's internal logic (safeguard, auto life level, etc.)
     */
    Error askBodyRotation(float x_rad, float y_rad, float z_rad);

    /**
     * @brief Request a body position (robot location).
     * @param x_m Position in the x direction (m)
     * @param y_m Position in the y direction (m)
     * @param z_m Position in the z direction (m)
     * @return Error code indicating success or failure of the operation.
     * @note This position will be applied or not depending on the Decision Loop's internal logic (safeguard, auto life level, etc.)
     */
    Error askBodyPosition(float x_m, float y_m, float z_m);

    /**
     * @brief Request a leg position (in hip frame).
     * @param leg_id The ID of the leg to control
     * @param x_m Position in the x direction (m)
     * @param y_m Position in the y direction (m)
     * @param z_m Position in the z direction (m)
     * @param mode The override mode to apply for this leg position command (default: Absolute)
     * @return Error code indicating success or failure of the operation.
     * @note This position will be applied or not depending on the Decision Loop's internal logic (safeguard, auto life level, etc.)
     */
    Error askLegPosition(Leg::Id leg_id, float x_m, float y_m, float z_m, IPC::OverrideMode mode = IPC::OverrideMode::Absolute);

    /**
     * @brief Request a gait type.
     * @param gait The gait type to request.
     * @return Error code indicating success or failure of the operation.
     * @note The requested gait will be applied or not depending on the Decision Loop's internal logic (safeguard, auto life level, etc.)
     */
    Error askGaitType(GaitPlanner::GaitType gait);

    /**
    * @brief Request a specific joint to be moved to a specific angle.
    * @param joint_id The ID of the joint to control.
    * @param angle_rad The target angle for the joint in radians.
    * @param mode The override mode to apply for this joint command (default: Absolute).
    * @return Error code indicating success or failure of the operation.
    * @note This joint command will be applied or not depending on the Decision Loop's internal logic (safeguard, auto life level, etc.)
    */
    Error askJointAngle(Joint::Id joint_id, float angle_rad, IPC::OverrideMode mode = IPC::OverrideMode::Absolute);

private:
    TaskHandle_t decision_loop_task = nullptr;
    bool loop_running = false;
    IPC::ControlIntent intent;
    IPC::RobotState state;
    uint8_t auto_life_level = AutoLifeLevel::Full; // Default to full auto life features enabled

    Vec3f askedBodyVel;
    Vec3f askedBodyRot;
    Vec3f askedBodyPos;
    GaitPlanner::GaitType askedGait = GaitPlanner::GaitType::Walk;
    IPC::LegOverride askedLegOverrides[(int) Leg::Id::Count];
    IPC::JointOverride askedJointAngles[(int) Joint::Id::Count];
    uint32_t last_ask_timestamp_ms = 0;

    /**
     * @brief Internal task for the main decision loop
     */
    void decision_loop();

    /**
     * @brief Update the decision loop's internal logic.
     * @param dt Time delta since last update in seconds
     * @param state Current robot state as received from the Reflex core
     * @note This is where the main decision logic is implemented (safeguard, auto life level features, etc.)
     */
    void update(float dt, const IPC::RobotState& state);

    /**
    * @brief Fill the intent object with the decision loop's control intentions.
    * @param intent The final intent object to be sent to the Reflex core. This object is modified in-place.
    * @param state The current robot state as received from the Reflex core. This is used to make decisions based on the current situation of the robot.
    * @note given intent is already filled with the asked intents, this function only override or modify them based on the decision loop's internal logic (safeguard, auto life level, etc.)
    */
    void fillIntent(IPC::ControlIntent& intent, const IPC::RobotState& state);
};
