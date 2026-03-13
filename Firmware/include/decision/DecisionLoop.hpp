#pragma once
#include <freertos/FreeRTOS.h>
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include "locomotion/IPC.hpp"

class DecisionLoop
{
public:
    constexpr static const char* TAG = "DecisionLoop";

    /**
     * @brief Initializes the Decision Loop module
     */
    Error init();

    /**
     * @brief Deinitializes the Decision Loop module
     */
    Error deinit();

    /**
     * @brief Starts the decision loop
     * @note This starts a new freertos task.
     */
    Error start();

    /**
     * @brief Stops the decision loop
     */
    Error stop();

    /**
     * @brief Set the body velocity
     * @note THIS IS TEMPORARY. PROPER API SHOULD COME SOON
     */
    void setBodyVelocity(Vec3f vel);

    /**
     * @brief Internal task for the main decision loop
     * @note SHOULDN'T BE CALLED
     */
    void decision_loop();

private:
    TaskHandle_t decision_loop_task = nullptr;
    bool loop_running = false;
    IPC::ControlIntent intent;
};
