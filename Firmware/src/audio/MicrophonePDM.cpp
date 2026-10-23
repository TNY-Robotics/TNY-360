#include "audio/MicrophonePDM.hpp"
#include "settings/Settings.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <algorithm>

Status MicrophonePDM::init()
{
    LOG_SCOPE(TAG, "MicrophonePDM::Init");
    
    sampleRate = Settings::GetConfig().audio.microphone.sampleRate;
    gain = Settings::GetConfig().audio.microphone.gain;

    i2s_chan_config_t i2s_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    if (esp_err_t err = i2s_new_channel(&i2s_config, nullptr, &i2s_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to create I2S channel with error: 0x%0x", err);
        return Status::Failure;
    }

    i2s_pdm_rx_config_t pdm_config = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(sampleRate),
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = static_cast<gpio_num_t>(Settings::GetConfig().audio.microphone.pdm.gpio_clk),
            .din = static_cast<gpio_num_t>(Settings::GetConfig().audio.microphone.pdm.gpio_data),
            .invert_flags = {
                .clk_inv = false,
            },
        }
    };

    if (esp_err_t err = i2s_channel_init_pdm_rx_mode(i2s_handle, &pdm_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to initialize I2S PDM RX with error: 0x%0x", err);
        i2s_del_channel(i2s_handle);
        i2s_handle = nullptr;
        return Status::Failure;
    }

    if (esp_err_t err = i2s_channel_enable(i2s_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to enable I2S channel with error: 0x%0x", err);
        i2s_del_channel(i2s_handle);
        i2s_handle = nullptr;
        return Status::Failure;
    }

    return Status::Ok;
}


Status MicrophonePDM::deinit()
{
    LOG_SCOPE(TAG, "MicrophonePDM::Deinit");
    
    Status status = Status::Ok;
    if (i2s_handle)
    {
        if (esp_err_t err = i2s_channel_disable(i2s_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to disable I2S channel with error: 0x%0x", err);
            status = Status::Failure;
        }

        if (esp_err_t err = i2s_del_channel(i2s_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to delete I2S channel with error: 0x%0x", err);
            status = Status::Failure;
        }
        i2s_handle = nullptr;
    }

    return status;
}

size_t MicrophonePDM::readSamples(Sample* buffer, size_t sampleCount)
{
    LOG_SCOPE(TAG, "MicrophonePDM::ReadSamples");

    if (!i2s_handle)
    {
        LOG_ERROR(TAG, "I2S channel is not initialized");
        return 0;
    }

    size_t bytes_read = 0;
    size_t bytes_requested = sampleCount * sizeof(Sample);

    // Ask samples with max timeout of 2x length of buffer (should be good? I don't want to put portMAX_DELAY ...)
    esp_err_t err = i2s_channel_read(i2s_handle, buffer, bytes_requested, &bytes_read, pdMS_TO_TICKS(sampleCount * 1000 * 2 / sampleRate));

    if (err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to read from I2S channel with error: 0x%0x", err);
        return 0;
    }
    if (bytes_read == 0)
    {
        LOG_WARNING(TAG, "No samples read from I2S channel");
        return 0;
    }

    // If gain isn't 1, scale (+ overflow protection)
    if (gain != 1)
    {
        size_t samples_read = bytes_read / sizeof(Sample);
        for (size_t i = 0; i < samples_read; ++i)
        {
            int32_t raw_sample = static_cast<int32_t>(buffer[i]);

            // slowly adjust the DC offset
            dc_offset += (raw_sample - dc_offset) / 64;

            // center + apply gain
            int32_t centered = raw_sample - dc_offset;
            int32_t amplified = centered * gain;

            // save with overflow protection
            buffer[i] = static_cast<Sample>(std::clamp(
                amplified,
                static_cast<int32_t>(std::numeric_limits<Sample>::min()), 
                static_cast<int32_t>(std::numeric_limits<Sample>::max())
            ));
        }
    }

    return bytes_read / sizeof(Sample);
}