#include "audio/MicrophoneI2S.hpp"
#include "settings/Settings.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <algorithm>
#include <limits>

Status MicrophoneI2S::init()
{
    LOG_SCOPE(TAG, "MicrophoneI2S::Init");

    sampleRate = Settings::GetConfig().audio.microphone.sampleRate;
    gain = Settings::GetConfig().audio.microphone.gain;

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    if (esp_err_t err = i2s_new_channel(&chan_cfg, nullptr, &i2s_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to create I2S channel with error: 0x%0x", err);
        return Status::Failure;
    }

    const auto& i2s_pins = Settings::GetConfig().audio.microphone.i2s;

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = static_cast<gpio_num_t>(i2s_pins.gpio_clk),
            .ws   = static_cast<gpio_num_t>(i2s_pins.gpio_ws),
            .dout = I2S_GPIO_UNUSED,
            .din  = static_cast<gpio_num_t>(i2s_pins.gpio_data),
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    // listening to the left channel only, as the microphone is mono (Microphones like the INMP441 output on the left channel)
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    if (esp_err_t err = i2s_channel_init_std_mode(i2s_handle, &std_cfg); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to initialize I2S standard RX mode with error: 0x%0x", err);
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

Status MicrophoneI2S::deinit()
{
    LOG_SCOPE(TAG, "MicrophoneI2S::Deinit");

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

size_t MicrophoneI2S::readSamples(Sample* buffer, size_t sampleCount)
{
    LOG_SCOPE(TAG, "MicrophoneI2S::ReadSamples");

    if (!i2s_handle)
    {
        LOG_ERROR(TAG, "I2S channel is not initialized");
        return 0;
    }

    size_t bytes_read = 0;
    size_t bytes_requested = sampleCount * sizeof(Sample);

    TickType_t timeout_ticks = pdMS_TO_TICKS((sampleCount * 1000 * 2) / sampleRate);
    esp_err_t err = i2s_channel_read(i2s_handle, buffer, bytes_requested, &bytes_read, timeout_ticks);

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

    // Gain and DC offset correction
    if (gain != 1)
    {
        size_t samples_read = bytes_read / sizeof(Sample);
        for (size_t i = 0; i < samples_read; ++i)
        {
            int32_t raw_sample = static_cast<int32_t>(buffer[i]);

            dc_offset += (raw_sample - dc_offset) / 64;

            int32_t centered = raw_sample - dc_offset;
            int32_t amplified = centered * gain;

            buffer[i] = static_cast<Sample>(std::clamp(
                amplified,
                static_cast<int32_t>(std::numeric_limits<Sample>::min()),
                static_cast<int32_t>(std::numeric_limits<Sample>::max())
            ));
        }
    }

    return bytes_read / sizeof(Sample);
}