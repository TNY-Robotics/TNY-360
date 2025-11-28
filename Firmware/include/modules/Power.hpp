#pragma once
#include "utils.hpp"

namespace Power
{
    /**
    * @brief Initializes the Power module.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Gets the current battery level.
    * @param[out] outLevel Pointer to store the battery level.
    * @return Error code indicating success or failure.
    */
    Error GetBatteryLevel(float* outLevel);

    /**
    * @brief Gets the current voltage.
    * @param[out] outVoltage Pointer to store the voltage.
    * @return Error code indicating success or failure.
    */
    Error GetVoltage(float* outVoltage);

    /**
    * @brief Gets the current flowing through the system.
    * @param[out] outCurrent Pointer to store the current.
    * @return Error code indicating success or failure.
    */
    Error GetCurrent(float* outCurrent);

    /**
    * @brief Checks if the device is charging.
    * @param[out] outIsCharging Pointer to store charging status.
    * @return Error code indicating success or failure.
    */
    Error IsCharging(bool* outIsCharging);
}