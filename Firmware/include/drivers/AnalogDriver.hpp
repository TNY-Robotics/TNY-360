#pragma once
#include "common/utils.hpp"

namespace AnalogDriver
{
    using Channel = uint8_t;
    using Value = float; // in Volt
    constexpr Channel CHANNEL_COUNT = 16;

    namespace internal
    {
        /**
         * @brief Selects the specified analog input channel using GPIO pins.
         * @param channel The channel number to select (0 to 15).
         * @return Error code indicating success or failure of the channel selection.
         * @note This function should not be called in normal usage.
         */
        Status select(Channel channel);

        /**
         * @brief Reads the voltage value from the currently selected analog input channel.
         * @param outVoltage Reference to store the read voltage value in Volt.
         * @return Error code indicating success or failure of the read operation.
         */
        Status read(Value& outVoltage);

        /**
         * @brief Reads the voltage value from the currently selected analog input channel using multiple samples for better accuracy.
         * @param outVoltage Reference to store the read voltage value in Volt.
         * @param nb_subsamples Number of subsamples to take and average.
         * @return Error code indicating success or failure of the read operation.
         */
        Status read_subsampled(Value& outVoltage, uint16_t nb_subsamples);
    }

    /**
    * @brief Initializes the Analog driver.
    * @return Error code indicating success or failure.
    */
    Status Init();

    /**
     * @brief Deinitializes the Analog driver.
     * @return Error code indicating success or failure.
     */
    Status Deinit();

    /**
    * @brief Gets the raw value of an analog input.
    * @param id Analog input identifier.
    * @param outVoltage Pointer to store the raw value.
    * @return Error code indicating success or failure.
    */
    Status GetVoltage(Channel id, Value& outVoltage);

    /**
    * @brief Gets all analog input voltages.
    * @param outVoltages Pointer to array to store voltage values.
    * @return Error code indicating success or failure.
    */
    Status GetVoltages(Value* outVoltages);

    /**
    * @brief Gets raw values for specified analog inputs.
    * @param ids Array of analog input identifiers.
    * @param outVoltages Pointer to array to store voltage values.
    * @param count Number of elements in the ids array.
    * @return Error code indicating success or failure.
    */
    Status GetVoltages(const Channel* ids, Value* outVoltages, uint8_t count);

    Status ReadAllChannels();
}