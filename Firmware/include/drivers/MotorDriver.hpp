#pragma once
#include "common/utils.hpp"
#include "common/config.hpp"

namespace MotorDriver
{
    using Channel = uint8_t;
    using Value = float;
    constexpr Channel CHANNEL_COUNT = 16;

    /**
     * @brief Converts duty cycle in milliseconds to the corresponding PWM value for the PCA9685.
     * @param dc Duty cycle in milliseconds (e.g., 1.0 for 1ms).
     * @return Corresponding PWM value (0-4096) for the PCA9685 at the defined frequency.
     */
    constexpr static Value DC_TO_PWM(float dc)
    {
        return (dc * 4096.f) * (MOTOR_DRIVER_PWM_FREQUENCY_HZ / 1000.f);
    }

    /**
     * @brief Converts a PWM value to the corresponding duty cycle in milliseconds for the PCA9685.
     * @param pwm PWM value (0-4096).
     * @return Corresponding duty cycle in milliseconds (e.g., 1.0 for 1ms).
     */
    constexpr static float PWM_TO_DC(Value pwm)
    {
        return (pwm / 4096.f) * (1000.f / MOTOR_DRIVER_PWM_FREQUENCY_HZ);
    }

    /**
    * @brief Initializes the Motor driver.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Deinitializes the Motor driver.
    * @return Error code indicating success or failure.
    */
    Error Deinit();

    /**
    * @brief Sets the duty cycle for a motor.
    * @param id Motor identifier.
    * @param duty_cycle Duty cycle value in milliseconds.
    * @return Error code.
    */
    Error SetDutyCycle(Channel id, Value duty_cycle);

    /**
     * @brief Gets the current duty cycle for a motor.
     * @param id Motor identifier.
     * @param duty_cycle Reference to store the duty cycle value in milliseconds.
     * @return Error code.
     */
    Error GetDutyCycle(Channel id, Value &duty_cycle);

    /**
     * @brief Disables all motors by setting their duty cycle to 0.
     * @return Error code.
     */
    Error DisableAllMotors();

    /**
     * @brief Internal function to send PWM values to PCA9685.
     * @note YOU SHOULD NOT CALL THIS FUNCTION DIRECTLY.
     * @return Error if send failed
     */
    Error SendData();
}