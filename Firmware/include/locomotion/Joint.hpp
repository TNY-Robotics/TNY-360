#pragma once
#include "locomotion/MotorController.hpp"
#include "common/KalmanFilter.hpp"
#include "common/geometry.hpp"
#include "common/config.hpp"

class Joint
{
public:
    constexpr static float MAX_VELOCITY_RAD_S = 5.0f; // Maximum velocity in radians per second
    constexpr static const char* TAG = "Joint";

    /**
     * @brief Get a Joint instance by its motor channel.
     * @param motor_channel The motor channel associated with the joint.
     * @return Pointer to the Joint instance, or nullptr if not found.
     */
    static Joint* GetJoint(MotorDriver::Channel motor_channel);

    /**
     * @brief Clamp maximum velocity for all joints.
     * @param max_velocity_rad_s Maximum velocity in radians per second. Set to 0 to disable clamping.
     * @return Error code indicating success or failure.
     */
    static Error ClampVelocity(float max_velocity_rad_s);

    Joint();

    Joint(MotorController motor_controller, float min_angle_rad = 0.f, float max_angle_rad = TWO_PI, bool inverted = false);

    /**
     * @brief Initialize the joint.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the joint.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Update the joint state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Error update();

    /**
     * @brief Enable the joint (motor).
     * @return Error code indicating success or failure.
     */
    Error enable();

    /**
     * @brief Disable the joint (motor).
     * @return Error code indicating success or failure.
     */
    Error disable();

    /**
     * @brief Check if the joint is enabled.
     * @return True if the joint is enabled, false otherwise.
     */
    bool isEnabled() const;

    /**
     * @brief Set the speed for the joint movement.
     * @param velocity_rad_s Velocity in radians per second.
     * @note Velocity is clamped to the range [0, MAX_VELOCITY_RAD_S].
     * @return Error code indicating success or failure.
     */
    Error setVelocity(float velocity_rad_s);

    /**
     * @brief Get the current velocity of the joint.
     * @param result Velocity in radians per second.
     * @return Error code indicating success or failure.
     */
    Error getVelocity(float &result) const;

    /**
     * @brief Set the target angle for the joint.
     * @param angle_rad Target angle in radians.
     * @return Error code indicating success or failure.
     */
    Error setTarget(float angle_rad);

    /**
     * @brief Get the target angle of the joint.
     * @param result Target angle in radians.
     * @return Error code indicating success or failure.
     */
    Error getTarget(float &result) const;

    /**
     * @brief Set the target angle for the joint with a specified time to reach it.
     * @param angle_rad Target angle in radians.
     * @param time_s Time in seconds to reach the target.
     * @return Error code indicating success or failure.
     */
    Error setTarget_Timed(float angle_rad, float time_s);

    /**
     * @brief Get the current position of the joint.
     * @param result Current angle in radians.
     * @note This method fuses feedback and model prediction for improved accuracy.
     * @return Error code indicating success or failure.
     */
    Error getPosition(float &result) const;

    /**
     * @brief Get the feedback angle of the joint.
     * @param result Feedback angle in radians.
     * @note Typical latency is 40ms between command and feedback.
     * @return Error code indicating success or failure.
     */
    Error getFeedback(float &result) const;

    /**
     * @brief Get the predicted angle of the joint (using joint model).
     * @param result Predicted angle in radians.
     * @note Fusion of feedback and model prediction is accessible using getPosition().
     * @return Error code indicating success or failure.
     */
    Error getPrediction(float &result) const;

    /**
     * @brief Get the uncertainty of the joint's angle.
     * @param result Uncertainty value.
     * @return Error code indicating success or failure.
     */
    Error getUncertainty(float &result) const;

    /**
     * @brief Return the estimated time to reach the given target from the current position.
     * @param target_angle_rad Target angle in radians.
     * @return Estimated time in seconds. (negative value if an error occurs)
     */
    float getTimeEstimate(float target_angle_rad) const;

    /**
     * @brief Get the underlying MotorController.
     * @return Reference to the MotorController.
     */
    MotorController& getMotorController() { return motor_controller; }

private:
    static Joint* joints[JOINT_COUNT]; // static array of all joints (index is motor channel)
    static float joint_velocity_clamp_rad_s; // static variable for global maximum joint velocity

    MotorController motor_controller;
    KalmanFilter1D kalman_filter;
    float min_angle_rad;
    float max_angle_rad;
    bool inverted;
    float target_angle_rad;
    float feedback_angle_rad;
    float estimate_angle_rad;
    float model_angle_rad;
    float velocity_rad_s;

    Error send_motorcontroller_position(const float& position);
    Error get_motorcontroller_position(float &result) const;
};