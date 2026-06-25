#pragma once
#include "locomotion/MotorController.hpp"
#include "common/KalmanFilter.hpp"
#include "common/geometry.hpp"
#include "common/config.hpp"

class Joint
{
public:
    constexpr static float MAX_VELOCITY_RAD_S = 1.0f; // Maximum velocity in radians per second
    constexpr static const char* TAG = "Joint";

    enum class Id : uint8_t
    {
        FrontLeftHipRoll = 0,
        FrontLeftHipPitch = 1,
        FrontLeftKneePitch = 2,
        BackLeftHipRoll = 3,
        BackLeftHipPitch = 4,
        BackLeftKneePitch = 5,
        BackRightHipRoll = 6,
        BackRightHipPitch = 7,
        BackRightKneePitch = 8,
        FrontRightHipRoll = 9,
        FrontRightHipPitch = 10,
        FrontRightKneePitch = 11,
        EarLeft = 12,
        EarRight = 13,
        Count = 14
    };

    /**
     * @brief Get a Joint instance by its ID.
     * @param id The ID of the joint.
     * @return Pointer to the Joint instance, or nullptr if not found.
     */
    static Joint* GetJoint(Joint::Id id);

    /**
     * @brief Clamp maximum velocity for all joints.
     * @param max_velocity_rad_s Maximum velocity in radians per second. Set to 0 to disable clamping.
     * @return Error code indicating success or failure.
     */
    static Status ClampVelocity(float max_velocity_rad_s);

    Joint();

    Joint(Joint::Id id, MotorController motor_controller, float min_angle_rad = 0.f, float max_angle_rad = TWO_PI, bool inverted = false, bool has_feedback = true);

    /**
     * @brief Initialize the joint.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the joint.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    /**
     * @brief Estimate the joint state.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Status estimateState(float dt);

    /**
     * @brief Apply a new command to the joint.
     * @note This method should not be called manually, it is called internally in the control loop.
     * @return Error code indicating success or failure.
     */
    Status applyCommand(float joint_angle_rad, float dt);

    /**
     * @brief Enable the joint (motor).
     * @return Error code indicating success or failure.
     * @note Useless if no feedback. Motor will be enabled when calling setTarget()
     */
    Status enable();

    /**
     * @brief Disable the joint (motor).
     * @return Error code indicating success or failure.
     */
    Status disable();

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
    Status setVelocity(float velocity_rad_s);

    /**
     * @brief Get the current velocity of the joint.
     * @param result Velocity in radians per second.
     * @return Error code indicating success or failure.
     */
    Status getVelocity(float &result) const;

    /**
     * @brief Get the target angle of the joint.
     * @param result Target angle in radians.
     * @return Error code indicating success or failure.
     */
    Status getTarget(float &result) const;

    /**
     * @brief Get the current position of the joint.
     * @param result Current angle in radians.
     * @note This method fuses feedback and model prediction for improved accuracy.
     * @return Error code indicating success or failure.
     */
    Status getPosition(float &result) const;

    /**
     * @brief Get the feedback angle of the joint.
     * @param result Feedback angle in radians.
     * @note Typical latency is 40ms between command and feedback.
     * @return Error code indicating success or failure.
     */
    Status getFeedback(float &result) const;

    /**
     * @brief Get the predicted angle of the joint (using joint model).
     * @param result Predicted angle in radians.
     * @note Fusion of feedback and model prediction is accessible using getPosition().
     * @return Error code indicating success or failure.
     */
    Status getPrediction(float &result) const;

    /**
     * @brief Get the uncertainty of the joint's angle.
     * @param result Uncertainty value.
     * @return Error code indicating success or failure.
     */
    Status getUncertainty(float &result) const;

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

    /**
     * @brief Get the minimum angle of the joint.
     * @return Minimum angle in radians.
     */
    float getMinAngle() const { return min_angle_rad; }

    /**
     * @brief Get the maximum angle of the joint.
     * @return Maximum angle in radians.
     */
    float getMaxAngle() const { return max_angle_rad; }

    /**
     * @brief Check if the joint is inverted.
     * @return True if the joint is inverted, false otherwise.
     */
    bool isInverted() const { return inverted; }

private:
    static Joint* joints[JOINT_COUNT]; // static array of all joints (index is motor channel)
    static float joint_velocity_clamp_rad_s; // static variable for global maximum joint velocity

    Joint::Id id;
    MotorController motor_controller;
    KalmanFilter1D kalman_filter;
    float min_angle_rad;
    float max_angle_rad;
    bool inverted;
    float target_angle_rad;
    float feedback_angle_rad = 0;
    float estimate_angle_rad = 0;
    float model_angle_rad = 0;
    float velocity_rad_s;

    Status send_motorcontroller_position(const float& position);
    Status get_motorcontroller_position(float &result) const;
};