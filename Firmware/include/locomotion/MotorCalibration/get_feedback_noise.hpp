#pragma once
#include <freertos/FreeRTOS.h>
#include "common/utils.hpp"
#include "common/analysis/ArrayStats.hpp"
#include "drivers/AnalogDriver.hpp"

struct FeedbackNoiseParams
{
    /// @brief Number of samples to use for noise detection
    uint16_t nb_samples = 20;
    /// @brief Number of subsamples to take for each sample
    uint16_t nb_subsamples = 10;
    /// @brief Wait delay between each noise sample
    uint16_t sample_delay_ms = 100;
};

/**
 * @brief Get a motor feedback noise in mV
 * @param params Noise detection parameters
 * @param channel AnalogDriver channel to sample
 * @param out_value [out] Final noise value output variable
 * @note Returned noise value is the standard deviation of the samples multiplied by 3 (covers 99% of values)
 */
Status get_feedback_noise(FeedbackNoiseParams params, AnalogDriver::Channel channel, AnalogDriver::Value& out_value)
{
    AnalogDriver::Value samples[params.nb_samples];

    // Read the voltages
    {
        AnalogDriver::internal::select(channel);
        vTaskDelay(pdMS_TO_TICKS(1)); // Short delay to ensure stabilization after channel switch
        for (uint16_t i = 0; i < params.nb_samples; i++)
        {
            RETURN_ON_ERROR(AnalogDriver::internal::read_subsampled(samples[i], params.nb_subsamples));
            vTaskDelay(pdMS_TO_TICKS(params.sample_delay_ms));
        }
    }

    // Compute noise value
    ArrayStats::Stats<AnalogDriver::Value> stats = ArrayStats::GetStats(samples, params.nb_samples);

    // Return result
    out_value = stats.std_dev * 3; // x3 : covers 99% of values
    return Status::Ok;
}