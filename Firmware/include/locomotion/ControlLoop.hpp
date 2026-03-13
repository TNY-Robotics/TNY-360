#pragma once
#include "common/utils.hpp"
#include "driver/gptimer.h"
#include "locomotion/MovementPlanner.hpp"

class ControlLoop
{
public:
    constexpr static const char* TAG = "ControlLoop";

    ControlLoop();

    /**
     * @brief Initializes the Control Loop.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitializes the Control Loop.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Starts the control loop 
     */
    Error start();

    /**
     * @brief Stops the control loop
     */
    Error stop();

    /**
     * @brief Internal Task for Control loop at 50 Hz.
     * @return Error code indicating success or failure.
     */
    Error control_task();
    
private:
    bool initialized;

    gptimer_handle_t timer = NULL;
    MovementPlanner movement_planner;
};