#pragma once
#include "drivers/MotorDriver.hpp"
#include "drivers/AnalogDriver.hpp"
#include "common/utils.hpp"
#include "common/NVS.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief MotorController class.
 * Uses MotorDriver, AnalogDriver and MotorCalibration to provide a 0-1 range position control and feedback for a motor.
 */
class MotorController
{
public:
    MotorController();
    
    /**
     * @brief Construct a MotorController instance.
     * @param joint_id The ID of the joint using this motor controller.
     * @param motor_channel The motor channel/index on the motor driver.
     * @param analog_channel The motor channel/index on the analog reader.
     */
    MotorController(uint8_t joint_id, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel);

    /**
     * @brief Initialize the motor controller.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the motor controller.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    /**
     * @brief Turns on the motor.
     * @return Error code indicating success or failure.
     * @note Will set the motor to the last target position set.
     * @note Call setTargetPosition() before enabling to set the initial position.
     */
    Status enable();

    /**
     * @brief Turns off the motor.
     * @return Error code indicating success or failure.
     */
    Status disable();

    /**
     * @brief Check if the motor is enabled.
     * @return True if the motor is enabled, false otherwise.
     */
    bool isEnabled() const;

    /**
     * @brief Set the target position for the motor.
     * @param position Target position as a ratio (0.0 to 1.0).
     * @return Error code indicating success or failure.
     */
    Status setTargetPosition(float position);

    /**
     * @brief Get the target position of the motor.
     * @return Target position as a ratio (0.0 to 1.0).
     */
    float getTargetPosition() const;

    /**
     * @brief Get the current position of the motor.
     * @return Current position as a ratio (0.0 to 1.0).
     */
    float getCurrentPosition() const;

    /**
     * @brief Get the motor channel.
     * @return Motor channel.
     */
    MotorDriver::Channel getMotorChannel() const { return motor_channel; }

    /**
     * @brief Get the analog channel.
     * @return Analog channel.
     */
    AnalogDriver::Channel getAnalogChannel() const { return analog_channel; }

private:
    uint8_t joint_id = 0;
    MotorDriver::Channel motor_channel;
    AnalogDriver::Channel analog_channel;
    
    float target = 0;
    bool enabled = false;

    Status __set_target_position();
};