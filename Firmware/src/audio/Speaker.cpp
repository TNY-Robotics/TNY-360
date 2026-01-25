#include "audio/Speaker.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <driver/i2s_pdm.h>
#include <vector>

i2s_chan_handle_t tx_handle = NULL;

Speaker::Speaker()
{
}

Error Speaker::init()
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 8;    // Number of DMA buffers
    chan_cfg.dma_frame_num = 512; // Size of each buffer
    if (esp_err_t err = i2s_new_channel(&chan_cfg, &tx_handle, NULL); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to create I2S channel: %s", esp_err_to_name(err));
        return Error::SoftwareFailure;
    }

    // Config PDM TX
    i2s_pdm_tx_config_t pdm_tx_cfg = {
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(SPEAKER_SAMPLE_RATE_HZ),
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = GPIO_NUM_NC, // No need
            .dout = SPEAKER_GPIO_NUM, // Data sound out
            .invert_flags = {
                .clk_inv = 0,
            },
        },
    };

    // Initialize PDM TX mode
    if (esp_err_t err = i2s_channel_init_pdm_tx_mode(tx_handle, &pdm_tx_cfg); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to init PDM TX mode: %s", esp_err_to_name(err));
        return Error::SoftwareFailure;
    }

    // Enable the channel
    if (esp_err_t err = i2s_channel_enable(tx_handle); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to enable I2S channel: %s", esp_err_to_name(err));
        return Error::SoftwareFailure;
    }

    return Error::None;
}

Error Speaker::deinit()
{
    return Error::None;
}

void Speaker::writeSamples(const int16_t* samples, size_t sampleCount)
{
    if (tx_handle == NULL)
    {
        Log::Add(Log::Level::Error, TAG, "I2S channel not initialized");
        return;
    }

    size_t bytesWritten = 0;
    size_t bytesToWrite = sampleCount * sizeof(int16_t);
    esp_err_t err = i2s_channel_write(tx_handle, (const void*)samples, bytesToWrite, &bytesWritten, portMAX_DELAY);
    if (err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to write samples to I2S channel: %s", esp_err_to_name(err));
    }
}