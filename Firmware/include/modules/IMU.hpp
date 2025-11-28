#pragma once
#include "utils.hpp"

namespace IMU
{
    using Callback = void(*)();

    /**
     * @brief Initializes the IMU module.
     * @return Error code indicating success or type of failure.
     */
    Error Init();

    /**
     * @brief Calibrates the IMU module.
     * @param onDone Callback function to be called when calibration is complete.
     * @return Error code indicating success or type of failure.
     */
    Error Calibrate(Callback onDone);

    /**
     * @brief Retrieves the current temperature from the IMU.
     * @param outTemperature Pointer to store the temperature value.
     * @return Error code indicating success or type of failure.
     */
    Error GetTemperature(float* outTemperature);

    // /**
    //  * @brief Retrieves the current orientation from the IMU.
    //  * @param outOrientation Pointer to store the orientation as a Quaternion.
    //  * @return Error code indicating success or type of failure.
    //  */
    // Error GetOrientation(Quaternion* outOrientation);

    /**
     * @brief Retrieves the current angular velocity from the IMU.
     * @param outAngularVelocity Pointer to store the angular velocity as a Vector3.
     * @return Error code indicating success or type of failure.
     */
    Error GetAngularVelocity(Vector3* outAngularVelocity);

    /**
     * @brief Retrieves the current linear acceleration from the IMU.
     * @param outLinearAcceleration Pointer to store the linear acceleration as a Vector3.
     * @return Error code indicating success or type of failure.
     */
    Error GetLinearAcceleration(Vector3* outLinearAcceleration);

    /**
     * @brief Checks if the IMU is currently calibrating.
     * @return True if calibrating, false otherwise.
     */
    bool isCalibrating();

    /**
     * @brief Gets the progress of the current calibration process.
     * @return A float between 0.0 and 1.0 indicating calibration progress.
     */
    float getCalibrationProgress();
}