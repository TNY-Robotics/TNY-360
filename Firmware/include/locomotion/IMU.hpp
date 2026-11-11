#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include "common/NVS.hpp"
#include <freertos/FreeRTOS.h>

class IMU
{
public:
    static constexpr const char* TAG = "IMU";
    static constexpr float GRAVITY = 9.80665f; // m/s^2

    /**
     * @brief Factory method to create an IMU instance based on the settings.
     * @return Pointer to the created IMU instance, or nullptr if disabled or creation failed.
     */
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
        Vec3f gyro_bias = Vec3f::Zero();
        /// @brief Scale factor to apply to the raw accelerometer readings to compensate for scale errors
        float accel_scale = 1.0f;
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
     * @note The orientation is represented as a quaternion (x, y, z, w) where w is the scalar part.
     */
    const Quatf& getOrientation() const { return orientation; }

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

    /// /// /// CALIBRATION

    /**
     * @brief Get the current calibration state of the IMU.
     * @return Current calibration state.
     */
    CalibrationState getCalibrationState() const { return calibrationState; }

    /**
     * @brief Get the current calibration data of the IMU.
     * @return Current calibration data.
     */
    CalibrationData getCalibrationData() const { return calibrationData; }

    /**
     * @brief Get the current calibration progress ratio (0.0 to 1.0).
     * @return Current calibration progress ratio.
     * @note This value is only valid when the calibration state is CALIBRATING.
     */
    float getCalibrationProgress() const { return calibrationProgress; }

    /**
     * @brief Start the calibration process.
     * @param save Whether to save the calibration data after the process is complete.
     * @return Status code indicating success or failure of the operation.
     * @note The robot should be stationary during calibration, with control loop disabled.
     * @note This function will return immediately, and the calibration process will run in the background.
     * @note Use getCalibrationProgress() and getCalibrationState() to monitor.
     */
    Status startCalibration(bool save = true);

    /**
     * @brief Stop the calibration process.
     * @return Status code indicating success or failure of the operation.
     */
    Status stopCalibration();

    /**
     * @brief Reset the calibration data to default values.
     * @param save Whether to save the reset calibration data after the operation.
     * @return Status code indicating success or failure of the operation.
     */
    Status resetCalibration(bool save = true);

protected:
    /**
     * @brief Internal override to provide IMU data to the controller.
     * @note This function is intended to be overridden by derived classes to provide actual IMU data.
     * @param accel Reference to a Vec3f object where the acceleration data will be stored (m/s^2).
     * @param angular_vel Reference to a Vec3f object where the angular velocity data will be stored (rad/s).
     * @return Status code indicating success or failure of the operation.
     */
    virtual Status provide_data(Vec3f &accel, Vec3f &angular_vel) = 0;

    /// NVS-Related functions for calibration data storage 
    Status init_nvs();
    Status load_calib_from_nvs();
    Status save_calib_to_nvs();
    Status delete_calib_from_nvs();

private:
    /// @brief Internal function to run the calibration sequence
    Status calibration_sequence();

    /// @brief Madgwick filter quaternion state
    Quatf q;
    /// @brief Madgwick filter gain
    float beta = 0.1f;

    /// @brief Current calibration data
    CalibrationData calibrationData;
    /// @brief Current calibration state
    CalibrationState calibrationState = CalibrationState::UNCALIBRATED;
    /// @brief Calibration progress ratio (0.0 to 1.0), valid only when calibrationState is CALIBRATING
    float calibrationProgress = 0.0f;
    /// @brief Calibration task handle for FreeRTOS
    TaskHandle_t calibrationTaskHandle = nullptr;
    /// @brief NVS Handle for storing calibration data
    NVS::Handle* nvsHandle = nullptr;

    /// @brief Gravity down vector in body frame
    Vec3f downVector = Vec3f(0.f, 0.f, -1.f);
    /// @brief Current orientation estimate
    Quatf orientation = Quatf(0.f, 0.f, 0.f, 1.f);
    /// @brief Current angular velocity in body frame (rad/s)
    Vec3f angularVelocity = Vec3f(0.f, 0.f, 0.f);
    /// @brief Current acceleration in body frame (m/s^2)
    Vec3f acceleration = Vec3f(0.f, 0.f, -GRAVITY);
};