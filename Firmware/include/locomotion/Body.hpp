#pragma once
#include "locomotion/Leg.hpp"
#include "locomotion/Joint.hpp"
#include "common/geometry.hpp"
#include "locomotion/IMUController.hpp"
#include "locomotion/ControlLoop.hpp"

class Body
{
public:
    constexpr static const char* TAG = "Body";

    Body();

    /**
     * @brief Initialize the body.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the body.
     * @return Error code indicating success or failure.
     */
    Status deinit();
    
    /**
     * @brief Estimate the body state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Status estimateState(float dt);

    /**
     * @brief Apply a new command to the entire body.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Status applyCommand(BodyJointState jointState, float dt);

    /**
     * @brief Enable all legs and ears of the body.
     * @return Error code indicating success or failure.
     */
    Status enable();

    /**
     * @brief Disable all legs and ears of the body.
     * @return Error code indicating success or failure.
     */
    Status disable();

    /**
     * @brief Enables all legs and ears of the body smoothly, slowing down the joints to avoid sudden movements.
     * @note This methods creates a new FreeRTOS task to handle the process. It will return immediately, run in background, and finish in [returnValue] seconds.
     * @return Time estimate for the smooth enable process in seconds, -1 if an error occurred.
     */
    float enableSmooth();

    /**
     * @brief Disables all legs and ears of the body smoothly, moving into a safe position before disabling the joints.
     * @note This methods creates a new FreeRTOS task to handle the process. It will return immediately, run in background, and finish in [returnValue] seconds.
     * @return Time estimate for the smooth disable process in seconds, -1 if an error occurred.
     */
    float disableSmooth();

    /**
     * @brief Get a given leg from its id
     * @param id The leg id, see Leg::Id.
     * @return Reference to the Leg.
     */
    inline Leg& getLeg(Leg::Id id) { return legs[static_cast<uint8_t>(id)]; }

    /**
     * @brief Get the front left leg.
     * @return Reference to the front left Leg.
     */
    inline Leg& getFrontLeftLeg() { return legs[static_cast<uint8_t>(Leg::Id::FrontLeft)]; }

    /**
     * @brief Get the back left leg.
     * @return Reference to the back left Leg.
     */
    inline Leg& getBackLeftLeg() { return legs[static_cast<uint8_t>(Leg::Id::BackLeft)]; }

    /**
     * @brief Get the back right leg.
     * @return Reference to the back right Leg.
     */
    inline Leg& getBackRightLeg() { return legs[static_cast<uint8_t>(Leg::Id::BackRight)]; }

    /**
     * @brief Get the front right leg.
     * @return Reference to the front right Leg.
     */
    inline Leg& getFrontRightLeg() { return legs[static_cast<uint8_t>(Leg::Id::FrontRight)]; }

    /**
     * @brief Get the left ear joint.
     * @return Reference to the left ear Joint.
     */
    inline Joint& getLeftEar() { return ear_l; }

    /**
     * @brief Get the right ear joint.
     * @return Reference to the right ear Joint.
     */
    inline Joint& getRightEar() { return ear_r; }

    /**
     * @brief Get the IMU controller.
     * @return Reference to the IMU controller.
     */
    inline IMUController& getIMUController() { return imu; }

private:
    Leg legs[4]; // Array of 4 legs (FL, BL, BR, FR order)
    Joint ear_l; // Left Ear Joint
    Joint ear_r; // Right Ear Joint

    IMUController imu;

    // Kinematic parameters and state
    Vec3f local_hip_positions_mm[4]; // local to base_link (body center)
    Vec3f global_feet_positions_mm[4]; // global positions in world frame (ground reference)
    Transformf posture; // Body posture (position relative to world, orientation relative to body frame)

    Status apply_posture();
};