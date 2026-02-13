#pragma once
#include "locomotion/Leg.hpp"
#include "locomotion/Joint.hpp"
#include "common/geometry.hpp"
#include "locomotion/IMUController.hpp"
#include "locomotion/MovementPlanner.hpp"

class Body
{
public:
    constexpr static const char* TAG = "Body";

    enum class LegIndex: uint8_t {
        FRONT_LEFT = 0,
        FRONT_RIGHT = 1,
        BACK_LEFT = 2,
        BACK_RIGHT = 3,
        COUNT = 4
    };

    Body();

    /**
     * @brief Initialize the body.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the body.
     * @return Error code indicating success or failure.
     */
    Error deinit();
    /**
     * @brief Update the body state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Error update();

    /**
     * @brief Enable all legs and ears of the body.
     * @return Error code indicating success or failure.
     */
    Error enable();

    /**
     * @brief Disable all legs and ears of the body.
     * @return Error code indicating success or failure.
     */
    Error disable();

    /**
     * @brief Starts the calibration process for all motors in the body.
     * @return Error code indicating success or failure.
     */
    Error startCalibration();

    /**
     * @brief Set the target position for the body.
     * @param target Target position for the body.
     * @return Error code indicating success or failure.
     */
    Error setPosture(const Transformf& posture);

    /**
     * @brief Get the current position of the body.
     * @param posture Current body posture.
     * @return Error code indicating success or failure.
     */
    Error getTargetPosture(Transformf& posture) const;

    /**
     * @brief Set the target position for a specific feet
     * @param leg_index Index of the leg (0-3).
     * @param target_pos Target foot position in world coordinate frame.
     * @return Error code indicating success or failure.
     */
    Error setFeetPosition(LegIndex leg_index, const Vec3f& position);

    /**
     * @brief Get the current position of a specific feet
     * @param leg_index Index of the leg (0-3).
     * @param target_pos Current foot position in world coordinate frame.
     * @return Error code indicating success or failure.
     */
    Error getFeetPosition(LegIndex leg_index, Vec3f& position) const;

    /**
     * @brief Get the front left leg.
     * @return Reference to the front left Leg.
     */
    Leg& getFrontLeftLeg() { return legs[static_cast<size_t>(LegIndex::FRONT_LEFT)]; }

    /**
     * @brief Get the front right leg.
     * @return Reference to the front right Leg.
     */
    Leg& getFrontRightLeg() { return legs[static_cast<size_t>(LegIndex::FRONT_RIGHT)]; }

    /**
     * @brief Get the back left leg.
     * @return Reference to the back left Leg.
     */
    Leg& getBackLeftLeg() { return legs[static_cast<size_t>(LegIndex::BACK_LEFT)]; }

    /**
     * @brief Get the back right leg.
     * @return Reference to the back right Leg.
     */
    Leg& getBackRightLeg() { return legs[static_cast<size_t>(LegIndex::BACK_RIGHT)]; }

    /**
     * @brief Get the left ear joint.
     * @return Reference to the left ear Joint.
     */
    Joint& getLeftEar() { return ear_l; }

    /**
     * @brief Get the right ear joint.
     * @return Reference to the right ear Joint.
     */
    Joint& getRightEar() { return ear_r; }

    /**
     * @brief Get the IMU controller.
     * @return Reference to the IMU controller.
     */
    IMUController& getIMUController() { return imu_controller; }

    /**
     * @brief Get the movement planner.
     * @return Reference to the movement planner.
     */
    MovementPlanner& getMovementPlanner() { return movement_planner; }

private:
    Leg legs[4]; // Array of 4 legs
    Joint ear_l; // Left Ear Joint
    Joint ear_r; // Right Ear Joint

    IMUController imu_controller; // IMU Controller

    // Movement planner, for walking and other complex movements
    MovementPlanner movement_planner;

    // Kinematic parameters and state
    Vec3f local_hip_positions_mm[4]; // local to base_link (body center)
    Vec3f global_feet_positions_mm[4]; // global positions in world frame (ground reference)
    Transformf posture; // Body posture (position relative to world, orientation relative to body frame)

    Error apply_posture();
};