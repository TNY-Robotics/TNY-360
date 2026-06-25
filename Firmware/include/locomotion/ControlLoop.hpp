#pragma once
#include "common/utils.hpp"
#include "driver/gptimer.h"
#include "locomotion/GaitPlanner.hpp"
#include "locomotion/KinematicsEngine.hpp"

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
     * @brief Get the control loop's GaitPlanner object.
     */
    inline GaitPlanner& getGaitPlanner() { return gait_planner; }

    /**
     * @brief Get the control loop's KinematicsEngine object.
     */
    inline KinematicsEngine& getKinematicsEngine() { return kinematics_engine; }

    /**
     * @brief Internal Task for Control loop at 50 Hz.
     * @return Error code indicating success or failure.
     */
    Status control_task();
    
private:
    Status create_internal_task();

    bool initialized;
    gptimer_handle_t timer = NULL;

    GaitPlanner gait_planner;
    KinematicsEngine kinematics_engine;
};