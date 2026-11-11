#pragma once
#include "common/utils.hpp"
#include "driver/gptimer.h"
#include "locomotion/providers/MotionProvider.hpp"

class ControlLoop
{
public:
    constexpr static const char* TAG = "ControlLoop";

    ControlLoop();

    /**
     * @brief Initializes the Control Loop.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitializes the Control Loop.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    /**
     * @brief Starts the control loop 
     */
    Status start();

    /**
     * @brief Stops the control loop
     */
    Status stop();

    /**
     * @brief Checks if the control loop is currently running.
     * @return true if the control loop is running, false otherwise.
     */
    bool isRunning() const;

    /**
     * @brief Sets the current motion provider to be used by the control loop.
     * @param provider Pointer to the motion provider to set.
     * @return Error code indicating success or failure.
     * @note The Control Loop takes ownership of the motion provider and will manage its lifecycle. Do not delete the motion provider after passing it to this function.
     */
    Status setMotionProvider(MotionProvider* provider);

    /**
     * @brief Gets the current motion provider used by the control loop.
     * @return Pointer to the current motion provider, nullptr if none is set.
     */
    MotionProvider* getMotionProvider() const { return motion_provider; }

    /**
     * @brief Internal Task for Control loop at 50 Hz.
     * @return Error code indicating success or failure.
     */
    Status control_task();
    
private:
    /// @brief Creates the internal task for the control loop.
    /// @return Error code indicating success or failure.
    Status create_internal_task();

    /// @brief Flag indicating if the control loop is initialized.
    bool initialized;
    /// @brief Control loop timer handle.
    gptimer_handle_t timer = NULL;

    /// @brief Flag indicating if the robot has fallen (and is in recovery mode).
    bool fallen = false;

    /// @brief Current motion provider in use by the control loop.
    MotionProvider* motion_provider = nullptr;
};