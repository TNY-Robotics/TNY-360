#pragma once
#include "locomotion/MotorController.hpp"
#include "common/KalmanFilter.hpp"
#include "common/geometry.hpp"
#include "common/config.hpp"
#include "settings/config.hpp"

class Joint
{
public:
    constexpr static float MAX_VELOCITY_RAD_S = 7.0f; // Maximum velocity in radians per second
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

    /// CLASS Joint

    Joint() = default;

    /**
     * @brief Construct a Joint instance.
     * @param id The ID of the joint.
     * @param config The configuration for the joint.
     */
    Joint(Joint::Id id, JointConfig config);

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
     * @note If no feedback is available (`getConfig().has_feedback == false`), the joint will only be enabled
     * on the next call to `applyCommand` with a valid target angle.
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
     * @brief Get the target angle of the joint.
     * @return Target angle in radians.
     */
    float getTargetAngle() const;

    /**
     * @brief Get the current angle of the joint.
     * @return Current angle in radians.
     * @note If the joint has no feedback (`getConfig().has_feedback == false`), a model approximation will be used instead.
     * @note This is just a getter. The actual current angle is updated during the estimateState() call.
     */
    float getCurrentAngle() const;

    /**
     * @brief Get the joint ID.
     * @return Joint ID.
     */
    Joint::Id getId() const;
    
    /**
     * @brief Get the joint configuration.
     * @return Joint configuration.
     */
    const JointConfig& getConfig() const;

private:
    static Joint* joints[static_cast<size_t>(Joint::Id::Count)];

    Joint::Id id;
    JointConfig config;
    
    KalmanFilter1D kalman_filter;
    MotorController motor_controller;
    
    bool enabled;
    float feedback_angle_rad;
    float model_angle_rad;
    float filtered_angle_rad;
    float target_angle_rad;

    Status set_motor_position(const float& position);
    Status get_motor_position(float &result) const;
};