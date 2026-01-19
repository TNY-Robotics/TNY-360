#pragma once
#include "locomotion/Leg.hpp"
#include "locomotion/Joint.hpp"
#include "common/geometry.hpp"

class Body
{
public:
    constexpr static const char* TAG = "Body";

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
     * @brief Get the front left leg.
     * @return Reference to the front left Leg.
     */
    Leg& getFrontLeftLeg() { return leg_fl; }

    /**
     * @brief Get the front right leg.
     * @return Reference to the front right Leg.
     */
    Leg& getFrontRightLeg() { return leg_fr; }

    /**
     * @brief Get the back left leg.
     * @return Reference to the back left Leg.
     */
    Leg& getBackLeftLeg() { return leg_bl; }

    /**
     * @brief Get the back right leg.
     * @return Reference to the back right Leg.
     */
    Leg& getBackRightLeg() { return leg_br; }

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

private:
    Leg leg_fl; // Front Left Leg
    Leg leg_fr; // Front Right Leg
    Leg leg_bl; // Back Left Leg
    Leg leg_br; // Back Right Leg
    Joint ear_l; // Left Ear Joint
    Joint ear_r; // Right Ear Joint
};