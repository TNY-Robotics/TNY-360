#pragma once
#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "common/utils.hpp"
#include "common/analysis/ArrayStats.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include <cmath>

struct DeadbandSizeParams
{
    /// @brief Duty cycle value to use to set servomotor to its center position
    MotorDriver::Value dc_center = 1.5;
    /// @brief Maximum duty cycle value to test (to avoid going too far in case of no feedback change)
    MotorDriver::Value max_dc = 2.5;
    /// @brief Motor's feedback noise
    AnalogDriver::Value feedback_noise;
    /// @brief Number of test for deadband size estimation
    uint16_t nb_samples = 5;
    /// @brief Number of subsamples to take for each sample
    uint16_t nb_subsamples = 40;
    /// @brief Wait delay after centering motor
    uint16_t base_delay_ms = 500;
    /// @brief Wait delay after each movement
    uint16_t move_delay_ms = 500;
};

/**
 * @brief Get a motor deadband size in PWM increments
 * @param params Deadband size estimation parameters
 * @param motor_channel Channel used for motor control
 * @param analog_channel Channel used for motor feedback
 * @param out_value [out] Final deadband size value in PWM increments
 * @note Returned value is the median of the sampled deadband size values
 */
Error get_deadband_size(DeadbandSizeParams params, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, float& out_value)
{
    AnalogDriver::internal::select(analog_channel);
    vTaskDelay(pdMS_TO_TICKS(1)); // Ensure stabilization

    // Move motor to center and wait a bit
    RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_center));
    RETURN_ERROR(MotorDriver::SendData());
    vTaskDelay(pdMS_TO_TICKS(params.base_delay_ms));

    MotorDriver::Value deadbands[params.nb_samples];
    MotorDriver::Value current_dc = params.dc_center;
    MotorDriver::Value last_dc = current_dc;
    AnalogDriver::Value last_feedback;
    int sample_index = 0;

    // get first feedback value
    RETURN_ERROR(AnalogDriver::internal::read_subsampled(last_feedback, params.nb_subsamples));

    // Gather all samples
    while (sample_index < params.nb_samples && current_dc < params.max_dc)
    {
        // Move the motor a bit and wait
        current_dc += 1;
        RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, current_dc));
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(params.move_delay_ms));

        // Get the current feedback value
        AnalogDriver::Value new_feedback;
        RETURN_ERROR(AnalogDriver::internal::read_subsampled(new_feedback, params.nb_subsamples));

        // If feedback has changed, record deadband size
        AnalogDriver::Value abs_diff = std::abs(new_feedback - last_feedback);
        if (abs_diff > params.feedback_noise) // Little margin to be sure
        {
            deadbands[sample_index] = current_dc - last_dc;
            last_feedback = new_feedback;
            last_dc = current_dc;
            sample_index++;
        }
    }

    if (sample_index < params.nb_samples)
    {
        LOG_ERROR("DB_SIZE", "Deadband size estimation failed: only %d valid samples collected (max_dc reached)", sample_index);
        return Error::OutOfBounds;
    }

    // Calculate deadband median (better than average in this case to mitigate outliers)
    ArrayStats::Stats<MotorDriver::Value> stats = ArrayStats::GetStats(deadbands, params.nb_samples);

    // Return result
    out_value = stats.median;
    return Error::None;
}