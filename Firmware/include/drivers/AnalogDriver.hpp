#pragma once
#include "common/utils.hpp"

namespace AnalogDriver
{
    using Channel = uint8_t;
    using Value = int; // in mV
    constexpr Channel CHANNEL_COUNT = 16;

    /**
    * @brief Initializes the AnalogDriver.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
     * @brief Deinitializes the AnalogDriver.
     * @return Error code indicating success or failure.
     */
    Error Deinit();

    /**
    * @brief Gets the raw value of an analog input.
    * @param id Analog input identifier.
    * @param outRawValue Pointer to store the raw value.
    * @return Error code indicating success or failure.
    */
    Error GetVoltage(Channel id, Value* outVoltage_mV);

    /**
    * @brief Gets all analog input voltages.
    * @param outVoltages_mV Pointer to array to store raw values.
    * @return Error code indicating success or failure.
    */
    Error GetVoltages(Value* outVoltages_mV);

    /**
    * @brief Gets raw values for specified analog inputs.
    * @param ids Array of analog input identifiers.
    * @param outVoltages_mV Pointer to array to store raw values.
    * @param count Number of elements in the ids array.
    * @return Error code indicating success or failure.
    */
    Error GetVoltages(const Channel* ids, Value* outVoltages_mV, uint8_t count);
    
    /**
     * @brief Internal update mhod for analog scanning.
     * @note YOU SHOULD NOT CALL THIS FUNCTION DIRECTLY.
     * @return void.
     */
    void __ISRStepRead();
}