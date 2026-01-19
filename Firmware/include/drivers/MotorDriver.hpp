#pragma once
#include "common/utils.hpp"

namespace MotorDriver
{
    using Channel = uint8_t;
    using Value = uint16_t;
    constexpr Channel CHANNEL_COUNT = 16;

    /**
    * @brief Initializes the Motor module.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Deinitializes the Motor module.
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
     * @brief Internal ISR function to send PWM values to PCA9685.
     * @note YOU SHOULD NOT CALL THIS FUNCTION DIRECTLY.
     * @return void.
     */
    void __ISRSendValues();
}