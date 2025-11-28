#pragma once
#include "utils.hpp"

namespace AnalogScanner
{
    enum class Id: uint8_t {
        Shoulder_FL = 0,
        Shoulder_FR,
        Shoulder_BL,
        Shoulder_BR,
        Leg_FL,
        Leg_FR,
        Leg_BL,
        Leg_BR,
        Knee_FL,
        Knee_FR,
        Knee_BL,
        Knee_BR,
        Pressure_FL,
        Pressure_FR,
        Pressure_BL,
        Pressure_BR,
        Count
    };

    /**
    * @brief Initializes the AnalogScanner module.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Calibrates an analog input.
    * @param id Analog input identifier.
    * @param raw_min Minimum raw value.
    * @param raw_max Maximum raw value.
    * @param real_min Minimum real-world value.
    * @param real_max Maximum real-world value.
    * @return Error code indicating success or failure.
    * @note Calibration data should be saved using SaveCalibration() to persist across reboots.
    */
    Error Calibrate(Id id, float raw_min, float raw_max, float real_min, float real_max);

    /**
     * @brief Saves the current calibration data to non-volatile storage.
     * @return Error code indicating success or failure.
     */
    Error SaveCalibration();

    /**
    * @brief Gets the calibrated value of an analog input.
    * @param id Analog input identifier.
    * @param outValue Pointer to store the value.
    * @return Error code indicating success or failure.
    */
    Error GetValue(Id id, float* outValue);

    /**
    * @brief Gets all calibrated analog values.
    * @param outValues Pointer to array to store values.
    * @return Error code indicating success or failure.
    */
    Error GetValues(float* outValues);

    /**
    * @brief Gets calibrated values for specified analog inputs.
    * @param ids Array of analog input identifiers.
    * @param outValues Pointer to array to store values.
    * @param count Number of elements in the ids array.
    * @return Error code indicating success or failure.
    */
    Error GetValues(const Id* ids, float* outValues, uint8_t count);

    /**
    * @brief Gets the raw value of an analog input.
    * @param id Analog input identifier.
    * @param outRawValue Pointer to store the raw value.
    * @return Error code indicating success or failure.
    */
    Error GetRawValue(Id id, int* outRawValue);

    /**
    * @brief Gets all raw analog values.
    * @param outRawValues Pointer to array to store raw values.
    * @return Error code indicating success or failure.
    */
    Error GetRawValues(int* outRawValues);

    /**
    * @brief Gets raw values for specified analog inputs.
    * @param ids Array of analog input identifiers.
    * @param outRawValues Pointer to array to store raw values.
    * @param count Number of elements in the ids array.
    * @return Error code indicating success or failure.
    */
    Error GetRawValues(const Id* ids, int* outRawValues, uint8_t count);
}