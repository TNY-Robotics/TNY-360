#pragma once
#include <freertos/FreeRTOS.h>
#include "common/utils.hpp"
#include "drivers/MotorDriver.hpp"
#include "drivers/AnalogDriver.hpp"
#include <cmath>

struct FeedbackInversionParams
{
    /// @brief Duty cycle value to use to set servomotor to its center position
    MotorDriver::Value dc_center = 1.5;
    /// @brief Duty cycle gap to apply to the center position to move the motor in each direction
    MotorDriver::Value dc_delta = 0.2;
    /// @brief Motor's feedback noise (to detect if dc_delta isn't big enough)
    AnalogDriver::Value feedback_noise;
    /// @brief Number of subsamples to take for each sample
    uint16_t nb_subsamples = 10;
    /// @brief Number of ms to wait after moving motor to center
    int base_delay_ms = 500;
    /// @brief Number of ms to wait between a movement and the feedback sampling
    int feedback_delay_ms = 200;
    /// @brief Number of ms to wait between forward and backward movement
    int movement_delay_ms = 100;
};

/**
 * @brief Check if motor feedback is inverted or not
 * @param params Feedback inversion parameters
 * @param motor_channel Channel used for motor control
 * @param analog_channel Channel used for motor feedback
 * @param out_value [out] Return value (true if inverted)
 * @note Invertion is detected by moving the motor forward and backward and checking resulting feedback
 */
Error check_feedback_inversion(FeedbackInversionParams params, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, bool& out_value)
{
    AnalogDriver::internal::select(analog_channel);
    vTaskDelay(pdMS_TO_TICKS(1)); // Ensure stabilization

    // Move the motor to center and wait a bit
    RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_center));
    RETURN_ERROR(MotorDriver::SendData());
    vTaskDelay(pdMS_TO_TICKS(params.base_delay_ms));

    // Move forward, wait a bit, and take feedback
    AnalogDriver::Value feedback_forward;
    RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_center + params.dc_delta));
    RETURN_ERROR(MotorDriver::SendData());
    vTaskDelay(pdMS_TO_TICKS(params.feedback_delay_ms));
    RETURN_ERROR(AnalogDriver::internal::read_subsampled(feedback_forward, params.nb_subsamples));
    vTaskDelay(pdMS_TO_TICKS(params.movement_delay_ms));

    // Move forward, wait a bit, and take feedback
    AnalogDriver::Value feedback_backward;
    RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_center - params.dc_delta));
    RETURN_ERROR(MotorDriver::SendData());
    vTaskDelay(pdMS_TO_TICKS(params.feedback_delay_ms));
    RETURN_ERROR(AnalogDriver::internal::read_subsampled(feedback_backward, params.nb_subsamples));
    vTaskDelay(pdMS_TO_TICKS(params.movement_delay_ms));

    // Compare feedbacks and return result
    AnalogDriver::Value abs_diff = std::abs(feedback_forward - feedback_backward);
    if (abs_diff <= params.feedback_noise)
    {
        LOG_ERROR("FDB_INV", "Feedback inversion test failed: no significant feedback change detected (abs diff: %f, noise: %f)", abs_diff, params.feedback_noise);
        return Error::HardwareFailure;
    }

    out_value = feedback_forward < feedback_backward;
    return Error::None;
}