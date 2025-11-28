#pragma once
#include "utils.hpp"

namespace Pressure
{
    using Callback = void(*)();
    enum class Id: uint8_t {
        FrontLeft = 0,
        FrontRight,
        BackLeft,
        BackRight,
        Count
    };

    /**
    * @brief Initializes the Pressure module.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Calibrates the Pressure module.
    * @param onDone Callback to be called when calibration is done.
    * @return Error code indicating success or failure.
    */
    Error Calibrate(Callback onDone);

    /**
    * @brief Gets the pressure for a specific sensor.
    * @param id Sensor ID.
    * @param outPressure Pointer to store the pressure value.
    * @return Error code indicating success or failure.
    */
    Error GetPressure(Id id, float* outPressure);

    /**
    * @brief Gets all pressure values.
    * @param outPressures Pointer to array to store pressure values.
    * @return Error code indicating success or failure.
    */
    Error GetPressures(float* outPressures);

    /**
    * @brief Gets pressure values for specific sensor IDs.
    * @param ids Array of sensor IDs.
    * @param outPressures Pointer to array to store pressure values.
    * @param count Number of sensor IDs.
    * @return Error code indicating success or failure.
    */
    Error GetPressures(const Id* ids, float* outPressures, uint8_t count);
}