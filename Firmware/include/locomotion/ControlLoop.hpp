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
     * @brief Get the average time takenfor one control loop iteration (in ms)
     * @return The time in milliseconds.
     * @note Compare to CONTROL_LOOP_DT_MS for load estimation
     */
    inline float getAvgTimeMs() { return avg_ms; }

    /**
     * @brief Internal Task for Control loop at 50 Hz.
     * @return Error code indicating success or failure.
     */
    Error control_task();
    
private:
    bool initialized;
    gptimer_handle_t timer = NULL;

    GaitPlanner gait_planner;
    KinematicsEngine kinematics_engine;

    PerfMonitor perf_controltask;
    uint16_t perf_counter = 0;
    float avg_ms;
};