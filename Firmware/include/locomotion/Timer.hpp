#pragma once
#include "common/utils.hpp"

class Timer
{
public:
    constexpr static const char* TAG = "Timer";

    Timer();

    /**
     * @brief Initializes the Timer module.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitializes the Timer module.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Internal Task for ADC readings at 800 Hz.
     * @return Error code indicating success or failure.
     */
    Error analog_read_task();

    /**
     * @brief Internal Task for Control loop at 50 Hz.
     * @return Error code indicating success or failure.
     */
    Error control_task();
    
private:
    bool initialized;
};