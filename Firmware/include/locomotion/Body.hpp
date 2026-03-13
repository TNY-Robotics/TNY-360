#pragma once
#include "locomotion/Leg.hpp"
#include "locomotion/Joint.hpp"
#include "common/geometry.hpp"
#include "locomotion/IMU.hpp"
#include "locomotion/ControlLoop.hpp"

class Body
{
public:
    constexpr static const char* TAG = "Body";

    /**
     * @brief Leg Index enum
     */
    enum class LegIndex: uint8_t {
        /// @brief  Front Left Leg
        FRONT_LEFT = 0,
        /// @brief  Front Right Leg
        FRONT_RIGHT = 1,
        /// @brief  Back Left Leg
        BACK_LEFT = 2,
        /// @brief  Back Right Leg
        BACK_RIGHT = 3,
        /// @brief  Total number of legs in the body
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
     * @brief Get a given leg from its index
     * @param index The leg index, see Body::LegIndex.
     * @return Reference to the Leg.
     */
    Leg& getLeg(LegIndex index) { return legs[static_cast<size_t>(index)]; }

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
    IMU& getIMU() { return imu; }

    /**
     * @brief Get the BOdy's Control Loop object
     * @return Reference to Control Loop's object
     */
    ControlLoop& getControlLoop() { return control_loop; }

private:
    Leg legs[4]; // Array of 4 legs
    Joint ear_l; // Left Ear Joint
    Joint ear_r; // Right Ear Joint

    IMU imu;
    
    ControlLoop control_loop;

    // Kinematic parameters and state
    Vec3f local_hip_positions_mm[4]; // local to base_link (body center)
    Vec3f global_feet_positions_mm[4]; // global positions in world frame (ground reference)
    Transformf posture; // Body posture (position relative to world, orientation relative to body frame)

    Error apply_posture();
};