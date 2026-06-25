#pragma once
#include "common/utils.hpp"

namespace PowerDriver
{
    constexpr const char* TAG = "PowerDriver";

    using Value = float;

    struct Data
    {
        Value voltage_v; // in volts
        Value current_a; // in amperes
        Value power_w;   // in watts
    };

    namespace internal
    {
        Status read_voltage(Value& voltage_v);
        Status read_current(Value& current_a);
        Status read_power(Value& power_w);
    }

    /**
    * @brief Initializes the Power driver.
    * @return Error code indicating success or failure.
    */
    Status Init();

    /**
    * @brief Deinitializes the Power driver.
    * @return Error code indicating success or failure.
    */
    Status Deinit();

    /**
     * @brief Internal function to read data from the INA219.
     * @note YOU SHOULD NOT CALL THIS FUNCTION DIRECTLY.
     * @return void.
     */
    Status ReadData();

    /**
     * @brief Get the last read values from the INA219.
     * @note You need to call ReadData() before to update the values.
     * @return most recent Data object.
     */
    Data& GetData();
}
