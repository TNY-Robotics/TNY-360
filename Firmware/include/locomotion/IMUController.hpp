#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"

class IMUController
{
public:
    enum class CalibrationState : uint8_t
    {
        UNCALIBRATED = 0,
        CALIBRATING = 1,
        CALIBRATED = 2,
        ERROR = 3,
    };

    /** <API_REF>
     * @type IMUController::CalibrationData
     * @desc Calibration data for the IMU controller.
     * @field gyro_bias Vec3f Bias to apply to the raw gyro readings to compensate for drift
     */
    struct CalibrationData
    {
        /// @brief Bias to apply to the raw gyro readings to compensate for drift
        Vec3f gyro_bias;
    };

    constexpr static const char* TAG = "IMUController";

    IMUController();

    /**
    * @brief Initializes the IMU controller.
    * @return Error code indicating success or failure.
    */
    Status init();

    /**
    * @brief Deinitializes the IMU controller.
    * @return Error code indicating success or failure.
    */
    Status deinit();

    /**
     * @brief Update the IMU controller state.
     * @param dt delta time in seconds
     * @return Error code indicating success or failure.
     */
    Status estimateState(float dt);

    /**
     * @brief Get the down vector representing gravity direction in body frame
     * @return Reference to the down vector.
     */
    Vec3f& getDownVector() { return downVector; }

    /**
     * @brief Get the current orientation estimate
     * @return Reference to the orientation quaternion.
     * @note Yaw is always zeroed, only pitch (y) and roll (x) are represented.
     */
    Vec3f& getOrientation() { return orientation; }

    /**
     * @brief Get the current angular velocity in body frame
     * @return Reference to the angular velocity vector (rad/s).
     */
    Vec3f& getAngularVelocity() { return angularVelocity; }

    /**
     * @brief Get the current acceleration in body frame
     * @return Reference to the acceleration vector (m/s^2).
     */
    Vec3f& getAcceleration() { return acceleration; }

private:
    /// @brief Gravity down vector in body frame
    Vec3f downVector;

    /// @brief Current orientation estimate (yaw is always zeroed, only pitch and roll used)
    Vec3f orientation;

    /// @brief Current angular velocity in body frame (rad/s)
    Vec3f angularVelocity;

    /// @brief Current acceleration in body frame (m/s^2)
    Vec3f acceleration;
};