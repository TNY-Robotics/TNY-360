#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"

class IMU
{
public:
    static constexpr const char* TAG = "IMU";
    static constexpr float GRAVITY = 9.80665f; // m/s^2

    static IMU* Create();

    enum class CalibrationState : uint8_t
    {
        UNCALIBRATED = 0,
        CALIBRATING = 1,
        CALIBRATED = 2,
        ERROR = 3,
    };

    /** <API_REF>
     * @type IMU::CalibrationData
     * @desc Calibration data for the IMU.
     * @field gyro_bias Vec3f Bias to apply to the raw gyro readings to compensate for drift
     */
    struct CalibrationData
    {
        /// @brief Bias to apply to the raw gyro readings to compensate for drift
        Vec3f gyro_bias;
    };

    IMU() = default;
    virtual ~IMU() = default;

    /**
    * @brief Initializes the IMU.
    * @return Error code indicating success or failure.
    */
    virtual Status init() = 0;

    /**
    * @brief Deinitializes the IMU.
    * @return Error code indicating success or failure.
    */
    virtual Status deinit() = 0;

    /**
     * @brief Update the IMU state.
     * @param dt delta time in seconds
     * @return Error code indicating success or failure.
     */
    Status estimateState(float dt);

    /**
     * @brief Get the down vector representing gravity direction in body frame
     * @return Reference to the down vector.
     */
    const Vec3f& getDownVector() const { return downVector; }

    /**
     * @brief Get the current orientation estimate
     * @return Reference to the orientation quaternion.
     * @note Yaw is always zeroed, only pitch (y) and roll (x) are represented.
     */
    const Vec3f& getOrientation() const { return orientation; }

    /**
     * @brief Get the current angular velocity in body frame
     * @return Reference to the angular velocity vector (rad/s).
     */
    const Vec3f& getAngularVelocity() const { return angularVelocity; }

    /**
     * @brief Get the current acceleration in body frame
     * @return Reference to the acceleration vector (m/s^2).
     */
    const Vec3f& getAcceleration() const { return acceleration; }

protected:
    /**
     * @brief Internal override to provide IMU data to the controller.
     * @note This function is intended to be overridden by derived classes to provide actual IMU data.
     * @param accel Reference to a Vec3f object where the acceleration data will be stored (m/s^2).
     * @param angular_vel Reference to a Vec3f object where the angular velocity data will be stored (rad/s).
     * @return Status code indicating success or failure of the operation.
     */
    virtual Status provide_data(Vec3f &accel, Vec3f &angular_vel) = 0;

private:
    /// @brief Gravity down vector in body frame
    Vec3f downVector = Vec3f(0.f, 0.f, -1.f);

    /// @brief Current orientation estimate (yaw is always zeroed, only pitch and roll used)
    Vec3f orientation = Vec3f(0.f, 0.f, 0.f);

    /// @brief Current angular velocity in body frame (rad/s)
    Vec3f angularVelocity = Vec3f(0.f, 0.f, 0.f);

    /// @brief Current acceleration in body frame (m/s^2)
    Vec3f acceleration = Vec3f(0.f, 0.f, -GRAVITY);
};