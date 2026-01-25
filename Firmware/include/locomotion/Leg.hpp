#pragma once
#include "locomotion/Joint.hpp"
#include "locomotion/LegKinematics.hpp"
#include "common/geometry.hpp"

class Leg
{
public:
    constexpr static const char* TAG = "Leg";

    Leg();

    Leg(Joint hip_roll, Joint hip_pitch, Joint knee_pitch, bool y_inverted = false);

    /**
     * @brief Initialize the leg.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the leg.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Update the leg state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Error update();

    /**
     * @brief Enable all joints of the leg.
     * @return Error code indicating success or failure.
     */
    Error enable();

    /**
     * @brief Disable all joints of the leg.
     * @return Error code indicating success or failure.
     */
    Error disable();

    /**
     * @brief Set the target position for the leg using inverse kinematics.
     * @param target_pos Target foot position in leg coordinate frame.
     * @return Error code indicating success or failure.
     */
    Error setTarget(const Vec3f& target_pos);

    /**
     * @brief Set the target position for the leg using inverse kinematics with a specified time to reach it.
     * @param target_pos Target foot position in leg coordinate frame.
     * @param time_s Time in seconds to reach the target.
     * @return Error code indicating success or failure.
     */
    Error setTarget_Timed(const Vec3f& target_pos, float time_s);

    /**
     * @brief Return the estimated time to reach the given target from the current position.
     * @param target_pos Target foot position in leg coordinate frame.
     * @return Estimated time in seconds. (negative value if an error occurs)
     */
    float getTimeEstimate(const Vec3f& target_pos) const;

    /**
     * @brief Get the hip roll joint.
     * @return Reference to the hip roll Joint.
     */
    Joint& getHipRoll() { return hip_roll; }

    /**
     * @brief Get the hip pitch joint.
     * @return Reference to the hip pitch Joint.
     */
    Joint& getHipPitch() { return hip_pitch; }

    /**
     * @brief Get the knee pitch joint.
     * @return Reference to the knee pitch Joint.
     */
    Joint& getKneePitch() { return knee_pitch; }

private:
    Joint hip_roll;
    Joint hip_pitch;
    Joint knee_pitch;
    bool y_inverted;

    Error calculate_ik_angles(const Vec3f& target_pos, LegAngles& angles) const;
};