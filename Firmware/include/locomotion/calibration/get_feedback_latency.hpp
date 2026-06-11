#pragma once
#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "common/utils.hpp"
#include "common/analysis/ArrayStats.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include <cmath>

constexpr int LATENCY_TIMEOUT_MS = 1000;

struct FeedbackLatencyParams
{
    /// @brief PWM Value to use to set servomotor to its center position
    MotorDriver::Value dc_center = 1.5;
    /// @brief Duty cycle gap to apply to the center position to see a feedback difference
    MotorDriver::Value dc_delta = 0.2;
    /// @brief Motor's feedback noise (to detect if dc_delta isn't big enough)
    AnalogDriver::Value feedback_noise;
    /// @brief Number of test for latency estimation
    uint16_t nb_samples = 10;
    /// @brief Number of subsamples to take for each sample
    uint16_t nb_subsamples = 20;
    /// @brief Wait delay after centering motor
    uint16_t base_delay_ms = 500;
    /// @brief Wait delay between each latency test
    uint16_t test_delay_ms = 500;
};

/**
 * @brief Get a motor feedback latency in ms
 * @param params Latency estimation parameters
 * @param motor_channel Channel used for motor control
 * @param analog_channel Channel used for motor feedback
 * @param out_value [out] Final latency value in milliseconds
 * @note Returned latency value is the average of the sampled latency values
 */
Error get_feedback_latency(FeedbackLatencyParams params, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, float& out_value)
{
    AnalogDriver::internal::select(analog_channel);
    vTaskDelay(pdMS_TO_TICKS(1)); // Ensure stabilization

    // Move motor to center and wait a bit
    RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_center));
    RETURN_ERROR(MotorDriver::SendData());
    vTaskDelay(pdMS_TO_TICKS(params.base_delay_ms));

    // Gather all samples
    int latencies[params.nb_samples];
    for (int i = 0; i < params.nb_samples; i++)
    {
        // Get the base feedback voltage
        AnalogDriver::Value base_feedback;
        RETURN_ERROR(AnalogDriver::internal::read_subsampled(base_feedback, params.nb_subsamples));

        // Get current time (now to take into account I2C latency + calculations) and move motor
        TickType_t start_tick = xTaskGetTickCount();
        RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_center + params.dc_delta));
        RETURN_ERROR(MotorDriver::SendData());
        // Loop until we see a change in feedback or timeout is reached
        TickType_t now_tick;
        AnalogDriver::Value new_feedback;
        do
        {
            // read feedback and get current time
            RETURN_ERROR(AnalogDriver::internal::read_subsampled(new_feedback, params.nb_subsamples));
            now_tick = xTaskGetTickCount();

            // If feedback has changed, record latency
            AnalogDriver::Value abs_diff = std::abs(new_feedback - base_feedback);
            if (abs_diff > params.feedback_noise) // Little margin to be sure
            {
                latencies[i] = now_tick - start_tick;
                break;
            }
        }
        while (start_tick + LATENCY_TIMEOUT_MS > now_tick);

        if (start_tick + LATENCY_TIMEOUT_MS <= now_tick)
        {
            // timeout, return error
            LOG_ERROR("FDB_LAT", "Latency test timeout: no feedback change detected after %d ms", LATENCY_TIMEOUT_MS);
            return Error::HardwareFailure;
        }

        // move to center, wait a bit and continue latency testing
        RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_center));
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(params.test_delay_ms));
    }

    // Calculate latency average 
    ArrayStats::Stats<int> stats = ArrayStats::GetStats(latencies, params.nb_samples);

    // Return result
    out_value = stats.mean;
    return Error::None;
}