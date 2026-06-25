#pragma once
#include "locomotion/Joint.hpp"
#include "locomotion/LegKinematics.hpp"
#include "locomotion/utils.hpp"
#include "common/geometry.hpp"

class Leg
{
public:
    constexpr static const char* TAG = "Leg";

    enum class Id : uint8_t
    {
        FrontLeft = 0,
        BackLeft = 1,
        BackRight = 2,
        FrontRight = 3,
        Count = 4
    };

    enum class JointId : uint8_t
    {
        HipRoll = 0,
        HipPitch = 1,
        KneePitch = 2,
        Count = 3
    };

    Leg();

    Leg(Joint hip_roll, Joint hip_pitch, Joint knee_pitch, AnalogDriver::Channel contact_channel, bool y_inverted = false);

    /**
     * @brief Initialize the leg.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the leg.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    /**
     * @brief Estimate the leg state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Status estimateState(float dt);

    /**
     * @brief Apply a new command.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Status applyCommand(LegJointState jointState, float dt);

    /**
     * @brief Enable all joints of the leg.
     * @return Error code indicating success or failure.
     */
    Status enable();

    /**
     * @brief Disable all joints of the leg.
     * @return Error code indicating success or failure.
     */
    Status disable();

    /**
     * @brief Get a joint of the leg by its ID (see JointId).
     * @param id The ID of the joint to retrieve.
     * @return Reference to the specified joint.
     */
    inline Joint& getJoint(JointId id)
    {
        return joints[(int)id];
    }

    /**
     * @brief Get the hip roll joint.
     * @return Reference to the hip roll Joint.
     */
    inline Joint& getHipRoll() { return joints[(int)JointId::HipRoll]; }

    /**
     * @brief Get the hip pitch joint.
     * @return Reference to the hip pitch Joint.
     */
    inline Joint& getHipPitch() { return joints[(int)JointId::HipPitch]; }

    /**
     * @brief Get the knee pitch joint.
     * @return Reference to the knee pitch Joint.
     */
    inline Joint& getKneePitch() { return joints[(int)JointId::KneePitch]; }

    /**
     * @brief Get if the leg is inverted along the y axis
     * @return Boolean true if inverted
     */
    inline bool isInverted() { return y_inverted; }

    /**
     * @brief Get if the leg touches ground or not
     * @return Boolean true if grounded
     */
    inline bool isGrounded() { return grounded; }

private:
    Joint joints[(int)JointId::Count];
    AnalogDriver::Channel contact_channel;
    bool y_inverted;

    bool grounded;
};