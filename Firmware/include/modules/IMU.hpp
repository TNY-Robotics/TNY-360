#pragma once
#include "utils.hpp"

namespace IMU
{
    using Callback = void(*)();

    /**
     * @brief Initializes the IMU module.
     * @return Error code indicating success or type of failure.
     */
    static Error Init();

    /**
     * @brief Calibrates the IMU module.
     * @param onDone Callback function to be called when calibration is complete.
     * @return Error code indicating success or type of failure.
     */
    static Error Calibrate(Callback onDone);

    /**
     * @brief Retrieves the current temperature from the IMU.
     * @param outTemperature Pointer to store the temperature value.
     * @return Error code indicating success or type of failure.
     */
    static Error GetTemperature(float* outTemperature);

    // /**
    //  * @brief Retrieves the current orientation from the IMU.
    //  * @param outOrientation Pointer to store the orientation as a Quaternion.
    //  * @return Error code indicating success or type of failure.
    //  */
    // static Error GetOrientation(Quaternion* outOrientation);

    /**
     * @brief Retrieves the current angular velocity from the IMU.
     * @param outAngularVelocity Pointer to store the angular velocity as a Vector3.
     * @return Error code indicating success or type of failure.
     */
    static Error GetAngularVelocity(Vector3* outAngularVelocity);

    /**
     * @brief Retrieves the current linear acceleration from the IMU.
     * @param outLinearAcceleration Pointer to store the linear acceleration as a Vector3.
     * @return Error code indicating success or type of failure.
     */
    static Error GetLinearAcceleration(Vector3* outLinearAcceleration);
}