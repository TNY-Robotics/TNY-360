#pragma once
#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "common/utils.hpp"
#include "common/analysis/ArrayStats.hpp"
#include "common/analysis/FastRegression.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include "drivers/PowerDriver.hpp"
#include <cmath>

struct PhysicalBoundParams
{
    /// @brief Duty Cycle Value to use to set servomotor to its center position
    MotorDriver::Value dc_center = 1.5;
    /// @brief Maximum Duty Cycle value to test (to avoid going too far)
    MotorDriver::Value dc_max = 2.6;
    /// @brief Minimum Duty Cycle value to test (to avoid going too far backwards)
    MotorDriver::Value dc_min = 0.4;
    /// @brief Safe zone in Duty Cycle value around the center position where we are sure to be within bounds 
    MotorDriver::Value dc_safe = 0.5;
    /// @brief Safe zone to use in slow approach fo the baseline
    MotorDriver::Value dc_safeslow = 0.2;
    /// @brief Duty Cycle step gap to use for the fast approach phase
    MotorDriver::Value dc_faststep = 0.01;
    /// @brief Duty Cycle step gap to use for the slow approach phase
    MotorDriver::Value dc_slowstep = 0.002;
    /// @brief Duty Cycle backoff to apply after detecting the bound with the fast approach, before starting the slow approach
    MotorDriver::Value dc_backoff = 0.2;
    /// @brief Direction in which to search for the bound (1 for max bound, -1 for min bound)
    int direction = 1;
    /// @brief Number of consecutive detections before confirming the bound (fast approach)
    uint32_t debounce_count_fast = 2;
    /// @brief Number of consecutive detections before confirming the bound (slow approach)
    uint32_t debounce_count_slow = 2;
    /// @brief Multiplier for the standard deviation to trigger FAST approach detection
    float snr_multiplier_fast = 6.0f;
    /// @brief Multiplier for the standard deviation to trigger SLOW approach detection
    float snr_multiplier_slow = 3.0f;
    /// @brief Multiplier for the standard deviation to define the true base of the slope during rollback analysis
    float snr_multiplier_base = 1.0f;
    /// @brief Base delay in ms for centering the motor at the start
    uint32_t base_delay_ms = 800;
    /// @brief Delay to wait between each step of the fast approach in ms
    uint32_t move_fast_delay_ms = 10;
    /// @brief Delay to wait between each step of the slow approach in ms
    uint32_t move_slow_delay_ms = 10;
    /// @brief Backoff delay in ms after detecting the bound with the fast approach
    uint32_t backoff_delay_ms = 300;
    /// @brief Number of subsamples to take for voltage feedback reading
    uint32_t feedback_subsamples = 40;
    /// @brief History buffer size for the rollback feature during slow approach
    uint32_t history_buffer_size = 40;
};

struct HistorySample {
    MotorDriver::Value dc;
    AnalogDriver::Value feedback;
    PowerDriver::Value current;
};

struct BoundDescription
{
    MotorDriver::Value dc_value; // Duty Cycle value at which the bound is detected
    AnalogDriver::Value feedback_value; // Feedback value at which the bound is detected
};

struct ProfilingParams
{
    MotorDriver::Value dc_start;
    MotorDriver::Value dc_end;
    MotorDriver::Value dc_step;
    uint32_t base_delay_ms;
    uint32_t step_delay_ms;
    int direction;
};

Error get_safezone_profiling(ProfilingParams params, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, PowerDriver::Value& out_current_baseline_avg, PowerDriver::Value& out_current_baseline_noise)
{
    int current_nb_samples = std::abs(params.dc_start - params.dc_end) / std::abs(params.dc_step);
    int current_samples_index = 0;

    PowerDriver::Value current_samples[current_nb_samples];
    // Send motor to the start of the evaluation range (safe zone)
    RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, params.dc_start));
    RETURN_ERROR(MotorDriver::SendData());
    vTaskDelay(pdMS_TO_TICKS(params.base_delay_ms));

    MotorDriver::Value current_eval_dc = params.dc_start;

    // Looping in the right direction (towards the bound)
    while ((params.direction == 1 && current_eval_dc <= params.dc_end) || (params.direction == -1 && current_eval_dc >= params.dc_end))
    {
        RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, current_eval_dc));
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(params.step_delay_ms));

        // read current
        PowerDriver::Value current;
        RETURN_ERROR(PowerDriver::internal::read_current(current));
        if (current_samples_index < current_nb_samples) // to avoid overflow in case of miscalculations and roundings
            current_samples[current_samples_index++] = current;
        
        current_eval_dc += params.dc_step;
    }

    // compute the current noise and average (with one motor moving freely in the safe zone)
    ArrayStats::Stats<PowerDriver::Value> current_stats = ArrayStats::GetStats(current_samples, current_nb_samples);
    out_current_baseline_avg = current_stats.mean; // This is the default current when the motor is not stalled
    out_current_baseline_noise = current_stats.std_dev; // We take x3 for 99% confidence interval
    return Error::None;
}

/**
 * @brief Get a physical bound (min or max) of a motor by moving it to the limit
 * @param params Physical bound estimation parameters
 * @param motor_channel Channel used for motor control
 * @param analog_channel Channel used for motor feedback
 * @param out_value [out] Description of the detected bound
 * @note Returned value is the median of the sampled deadband size values
 */
Error get_physical_bound(PhysicalBoundParams params, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, BoundDescription& out_value)
{   
    AnalogDriver::internal::select(analog_channel);
    vTaskDelay(pdMS_TO_TICKS(1));

    if (params.dc_faststep <= 0)
    {
        LOG_DEBUG("GET_PHC_BND", "Invalid fast step, defaulting to 1 pwm unit");
        params.dc_faststep = MotorDriver::PWM_TO_DC(1);
    }
    if (params.dc_slowstep <= 0)
    {
        LOG_DEBUG("GET_PHC_BND", "Invalid slow step, defaulting to 1 pwm unit");
        params.dc_slowstep = MotorDriver::PWM_TO_DC(1);
    }

    // =========================================================================
    // STEP 0 : Profiling inside Safe Zone
    // =========================================================================
    MotorDriver::Value eval_start = params.dc_center - (params.direction * params.dc_safe);
    MotorDriver::Value eval_end = params.dc_center + (params.direction * params.dc_safe);
    MotorDriver::Value eval_step = params.direction * params.dc_faststep;

    PowerDriver::Value current_baseline_avg, current_baseline_noise;
    RETURN_ERROR(get_safezone_profiling(
        {eval_start, eval_end, eval_step, params.base_delay_ms, params.move_fast_delay_ms, params.direction},
        motor_channel, analog_channel, current_baseline_avg, current_baseline_noise
    ));
    MotorDriver::Value current_dc = eval_end;

    // =========================================================================
    // STEP 1 : Fast Approach
    // =========================================================================
    {
        float fast_threshold = current_baseline_avg + current_baseline_noise * params.snr_multiplier_fast;

        int stall_debounce_count = 0;
        bool bound_found = false;
        while ((params.direction == 1 && current_dc <= params.dc_max) || (params.direction == -1 && current_dc >= params.dc_min))
        {
            current_dc += params.direction * params.dc_faststep;
            RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, current_dc));
            RETURN_ERROR(MotorDriver::SendData());
            vTaskDelay(pdMS_TO_TICKS(params.move_fast_delay_ms));

            // read current
            PowerDriver::Value current;
            RETURN_ERROR(PowerDriver::internal::read_current(current));

            // Check if current is above the threshold, in which case increment the debounce counter, otherwise reset it
            if (current > fast_threshold)
            {
                stall_debounce_count++;
                if (stall_debounce_count >= params.debounce_count_fast) {
                    bound_found = true;
                    break;
                }
            } else stall_debounce_count = 0;
        }

        if (!bound_found) {
            LOG_ERROR("GET_PHC_BND", "Reached Duty Cycle limit (%f) without finding bound. Check motor assembly.", params.direction == 1 ? params.dc_max : params.dc_min);
            return Error::HardwareFailure;
        }
    }

    // =========================================================================
    // Step 2 : Back-off
    // =========================================================================
    {
        current_dc -= (params.direction * params.dc_backoff);
        RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, current_dc));
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(params.backoff_delay_ms));
    }
    
    // =========================================================================
    // STEP 0 : Profiling inside Safe Zone
    // =========================================================================
    eval_start = current_dc - (params.direction * params.dc_safeslow);
    eval_end = current_dc;
    eval_step = params.direction * params.dc_slowstep;

    RETURN_ERROR(get_safezone_profiling(
        {eval_start, eval_end, eval_step, params.base_delay_ms, params.move_slow_delay_ms, params.direction},
        motor_channel, analog_channel, current_baseline_avg, current_baseline_noise
    ));
    current_dc = eval_end;

    // =========================================================================
    // Step 4 : Precision Touch + Rollback
    // =========================================================================
    {
        float slow_threshold = current_baseline_avg + current_baseline_noise * params.snr_multiplier_slow;

        int stall_debounce_count = 0;
        bool bound_found = false;

        HistorySample history[params.history_buffer_size];
        uint32_t history_idx = 0;
        uint32_t samples_collected = 0;

        while ((params.direction == 1 && current_dc <= params.dc_max) || (params.direction == -1 && current_dc >= params.dc_min))
        {
            current_dc += params.direction * params.dc_slowstep;
            RETURN_ERROR(MotorDriver::SetDutyCycle(motor_channel, current_dc));
            RETURN_ERROR(MotorDriver::SendData());
            vTaskDelay(pdMS_TO_TICKS(params.move_slow_delay_ms));

            // Read feedback 
            AnalogDriver::Value feedback;
            RETURN_ERROR(AnalogDriver::internal::read_subsampled(feedback, params.feedback_subsamples));

            // read current
            PowerDriver::Value current;
            RETURN_ERROR(PowerDriver::internal::read_current(current));

            // Save in rollback history
            history[history_idx].dc = current_dc;
            history[history_idx].feedback = feedback;
            history[history_idx].current = current;

            history_idx = (history_idx + 1) % params.history_buffer_size;
            if (samples_collected < params.history_buffer_size) samples_collected++;

            // Check if current is above the threshold, in which case increment the debounce counter, otherwise reset it
            if (current > slow_threshold)
            {
                stall_debounce_count++;
                if (stall_debounce_count >= params.debounce_count_slow) {
                    bound_found = true;
                    break;
                }
            } else stall_debounce_count = 0;
        }

        if (!bound_found) {
            LOG_ERROR("GET_PHC_BND", "Reached Duty Cycle limit (%f) without finding bound. Check motor assembly.", params.direction == 1 ? params.dc_max : params.dc_min);
            return Error::HardwareFailure;
        }

        // Disable motor to avoid stressing it
        RETURN_ERROR(MotorDriver::DisableAllMotors());

        // Now rollback to find the exact moment where the bound is touched
        {
            float base_threshold = current_baseline_avg + current_baseline_noise * params.snr_multiplier_base;

            bool origin_found = false;
        
            // Go back in the buffer (most recent to oldest)
            for (int i = 1; i <= samples_collected; i++) {
                // Precent index in the circular buffer
                int check_idx = (history_idx - i + params.history_buffer_size) % params.history_buffer_size;
                
                // Just before the impact is where the current goes back under the base threshold
                if (history[check_idx].current < base_threshold) {
                    out_value.dc_value = history[check_idx].dc;
                    out_value.feedback_value = history[check_idx].feedback;
                    origin_found = true;                    
                    break;
                }
            }

            // If the buffer was too small (couldn't find the base of the slope)
            if (!origin_found) {
                LOG_WARNING("GET_PHC_BND", "Could not find the base of the slope in history. Using oldest sample.");
                // Use the oldest sample in the buffer as a fallback (least accurate but better than nothing)
                int oldest_idx = (history_idx - samples_collected + params.history_buffer_size) % params.history_buffer_size;
                out_value.dc_value = history[oldest_idx].dc;
                out_value.feedback_value = history[oldest_idx].feedback;
            }
        }
    }

    return Error::None;
}