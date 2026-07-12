#pragma once
#include "common/utils.hpp"

/// @brief This namespace contains functions and utilities for diagnostic operations on the robot.
namespace Diagnostic
{
    /**
     * @brief Initializes the diagnostic system.
     * @note This function should be called before using any other diagnostic functions.
     * @returns Status::Ok if the operation was successful, or an appropriate error status otherwise.
     */
    Status Init();
    
    /**
     * @brief Deinitializes the diagnostic system.
     * @note This function should be called when the diagnostic system is no longer needed, to free up resources.
     * @returns Status::Ok if the operation was successful, or an appropriate error status otherwise.
     */
    Status Denit();

    /**
     * @brief Sets the diagnostic mode state.
     * @note This function should not be used directly to toggle diagnostic mode. Use the RebootInDiagnosticMode() function to reboot into diagnostic mode instead.
     * @param enable true to enable diagnostic mode, false to disable.
     */
    void SetDiagnosticMode(bool enable);

    /**
     * @brief Reboots the robot into diagnostic mode.
     * @note This function will set the necessary flags in NVS to ensure that the robot boots into diagnostic mode on the next restart.
     * @returns Status::Ok if the operation was successful, or an appropriate error status otherwise.
     */
    Status RebootInDiagnosticMode();

    /**
     * @brief Checks if the robot is currently in diagnostic mode.
     * @returns true if diagnostic mode is enabled, false otherwise.
     */
    bool IsDiagnosticModeEnabled();

    namespace Check
    {
        /// @brief Checks the status of the LED module.
        /// @returns 0 if the LED module is functioning correctly, or an error ID otherwise
        uint8_t LED();
        /// @brief Checks the status of the I2C module.
        /// @return 0 if the I2C module is functioning correctly, or an error ID otherwise
        uint8_t I2C();
        /// @brief Checks the status of the Speaker module.
        /// @returns 0 if the Speaker module is functioning correctly, or an error ID otherwise
        uint8_t Speaker();
        /// @brief Checks the status of the Microphone module.
        /// @returns 0 if the Microphone module is functioning correctly, or an error ID otherwise
        uint8_t Microphone();
        /// @brief Checks the status of the Camera module.
        /// @returns 0 if the Camera module is functioning correctly, or an error ID otherwise
        uint8_t Camera();
        /// @brief Checks the status of the Laser module.
        /// @returns 0 if the Laser module is functioning correctly, or an error ID otherwise
        uint8_t Laser();
        /// @brief Checks the status of the IMU module.
        /// @returns 0 if the IMU module is functioning correctly, or an error ID otherwise
        uint8_t IMU();
        /// @brief Checks the status of the Power module.
        /// @returns 0 if the Power module is functioning correctly, or an error ID otherwise
        uint8_t Power();
        /// @brief Checks the status of the Motor Driver module.
        /// @returns 0 if the Motor Driver module is functioning correctly, or an error ID otherwise
        uint8_t MotorDriver();
        /// @brief Checks the status of the Analog Reader module.
        /// @returns 0 if the Analog Reader module is functioning correctly, or an error ID otherwise
        uint8_t AnalogReader();
        /// @brief Checks the status of the Control Loop module.
        /// @returns 0 if the Control Loop module is functioning correctly, or an error ID otherwise
        uint8_t ControlLoop();
        /// @brief Checks the status of the Decision Loop module.
        /// @returns 0 if the Decision Loop module is functioning correctly, or an error ID otherwise
        uint8_t DecisionLoop();
    }
}