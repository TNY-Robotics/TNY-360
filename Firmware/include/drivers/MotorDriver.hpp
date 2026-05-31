#pragma once
#include "common/utils.hpp"
#include "common/config.hpp"

namespace MotorDriver
{
    using Channel = uint8_t;
    using Value = int;
    constexpr Channel CHANNEL_COUNT = 16;

    /**
     * @brief Converts milliseconds to the corresponding PWM value for the PCA9685.
     * @param ms Time in milliseconds (e.g., 1.0 for 1ms).
     * @return Corresponding PWM value (0-4096) for the PCA9685 at the defined frequency.
     */
    constexpr static Value MS_TO_PWM(float ms)
    {
        return 4096.f * ms / (1000.f / (float)MOTOR_DRIVER_PWM_FREQUENCY_HZ);
    }

    /**
     * @brief Converts a PWM value to the corresponding time in milliseconds for the PCA9685.
     * @param pwm PWM value (0-4096).
     * @return Corresponding time in milliseconds (e.g., 1.0 for 1ms).
     */
    constexpr static float PWM_TO_MS(Value pwm)
    {
        return pwm * (1000.f / (float)MOTOR_DRIVER_PWM_FREQUENCY_HZ) / 4096.f;
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
    * @brief Sets the PWM value for a motor.
    * @param id Motor identifier.
    * @param pwm PWM value (0-4096).
    * @return Error code.
    */
    Error SetPWM(Channel id, Value pwm);

    /**
     * @brief Gets the current PWM value for a motor.
     * @param id Motor identifier.
     * @param pwm Reference to store the PWM value (0-4096).
     * @return Error code.
     */
    Error GetPWM(Channel id, Value &pwm);

    /**
     * @brief Disables all motors by setting their PWM to 0.
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