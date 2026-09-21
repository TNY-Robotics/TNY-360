#include "audio/SpeakerI2S.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "common/Error.hpp"
#include "settings/Settings.hpp"
#include "audio/Speaker.Error.hpp"
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>

SpeakerI2S::SpeakerI2S()
{
}

Status SpeakerI2S::init()
{
    LOG_SCOPE(TAG, "SpeakerI2S::Init");

    if (txChannel != nullptr)
    {
        return Status::Ok;
    }

    i2s_chan_config_t channelConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    esp_err_t err = i2s_new_channel(&channelConfig, &txChannel, nullptr);
    if (err != ESP_OK)
    {
        txChannel = nullptr;
        Error::RegisterErrorEvent(ErrorEventCreateI2SFailed(err));
        return Status::Failure;
    }

    i2s_std_config_t stdConfig = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(static_cast<uint32_t>(Settings::GetConfig().audio.sampleRate)),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = static_cast<gpio_num_t>(Settings::GetConfig().audio.i2s.gpio_clk),
            .ws = static_cast<gpio_num_t>(Settings::GetConfig().audio.i2s.gpio_ws),
            .dout = static_cast<gpio_num_t>(Settings::GetConfig().audio.i2s.gpio_data),
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    stdConfig.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;

    err = i2s_channel_init_std_mode(txChannel, &stdConfig);
    if (err != ESP_OK)
    {
        Error::RegisterErrorEvent(ErrorEventInitPDMFailed(err));
        i2s_del_channel(txChannel);
        txChannel = nullptr;
        return Status::Failure;
    }

    err = i2s_channel_enable(txChannel);
    if (err != ESP_OK)
    {
        Error::RegisterErrorEvent(ErrorEventEnableI2SFailed(err));
        i2s_del_channel(txChannel);
        txChannel = nullptr;
        return Status::Failure;
    }

    return Status::Ok;
}

Status SpeakerI2S::deinit()
{
    if (txChannel == nullptr)
    {
        return Status::Ok;
    }

    esp_err_t err = i2s_channel_disable(txChannel);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        Error::RegisterErrorEvent(ErrorEventDisableI2SFailed(err));
        return Status::Failure;
    }

    err = i2s_del_channel(txChannel);
    if (err != ESP_OK)
    {
        Error::RegisterErrorEvent(ErrorEventDeleteI2SFailed(err));
        return Status::Failure;
    }

    txChannel = nullptr;

    return Status::Ok;
}

void SpeakerI2S::writeSamples(const int16_t* samples, size_t sampleCount)
{
    if (txChannel == nullptr || samples == nullptr || sampleCount == 0)
    {
        return;
    }

    size_t bytesWritten = 0;
    i2s_channel_write(txChannel, samples, sampleCount * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
}