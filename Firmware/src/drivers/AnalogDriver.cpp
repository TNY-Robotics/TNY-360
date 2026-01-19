#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include "drivers/AnalogDriver.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include <vector>
#include <algorithm>

namespace AnalogDriver
{
    constexpr const char* TAG = "AnalogDriver";

    static bool initialized = false;
    static adc_oneshot_unit_handle_t adc_handle = nullptr;
    static adc_cali_handle_t cali_handle = nullptr;

    static Value voltages_buffer[static_cast<size_t>(CHANNEL_COUNT)] = { 0 };

    static Channel update_current_channel = 0;
    
    Error select(Channel channel)
    {
        if (gpio_set_level(SCANNER_SLCT_PIN1, (channel & 0b0001) >> 0) != ESP_OK ||
            gpio_set_level(SCANNER_SLCT_PIN2, (channel & 0b0010) >> 1) != ESP_OK ||
            gpio_set_level(SCANNER_SLCT_PIN3, (channel & 0b0100) >> 2) != ESP_OK ||
            gpio_set_level(SCANNER_SLCT_PIN4, (channel & 0b1000) >> 3) != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to select index with GPIO");
            return Error::Unknown;
        }
        return Error::None;
    }

    Error Init()
    {
        if (initialized)
            return Error::None;

        // Setup select pins
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << SCANNER_SLCT_PIN1) | (1ULL << SCANNER_SLCT_PIN2) | (1ULL << SCANNER_SLCT_PIN3) | (1ULL << SCANNER_SLCT_PIN4);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        if (gpio_config(&io_conf) != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to configure GPIO for select pins");
            return Error::Unknown;
        }

        // Create adc oneshot handle
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
            .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
        if (adc_oneshot_new_unit(&init_config, &adc_handle) != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to create ADC oneshot handle");
            return Error::Unknown;
        }

        // Configure adc channel
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        if (adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_1, &config) != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to configure ADC oneshot channel");
            return Error::Unknown;
        }

        // configure calibration handle
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .chan = ADC_CHANNEL_1,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);

        // select initial channel
        select(update_current_channel); // 0

        initialized = true;
        return Error::None;
    }

    Error Deinit()
    {
        if (!initialized)
            return Error::None;

        adc_oneshot_del_unit(adc_handle);
        adc_handle = nullptr;

        adc_cali_delete_scheme_curve_fitting(cali_handle);
        cali_handle = nullptr;

        initialized = false;
        return Error::None;
    }

    Error GetVoltage(Channel id, Value* outVoltage_mV)
    {
        if (id >= CHANNEL_COUNT)
        {
            Log::Add(Log::Level::Error, TAG, "GetVoltage: Invalid channel id %d", id);
            return Error::InvalidParameters;
        }

        *outVoltage_mV = voltages_buffer[id];
        return Error::None;
    }

    Error GetVoltages(Value* outVoltages_mV)
    {
        for (size_t i = 0; i < static_cast<size_t>(CHANNEL_COUNT); i++)
        {
            outVoltages_mV[i] = voltages_buffer[i];
        }
        return Error::None;
    }

    Error GetVoltages(const Channel* ids, Value* outVoltages_mV, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            Channel id = ids[i];
            if (id >= CHANNEL_COUNT)
            {
                Log::Add(Log::Level::Error, TAG, "GetVoltages: Invalid channel id %d", id);
                return Error::InvalidParameters;
            }
            outVoltages_mV[i] = voltages_buffer[id];
        }
        return Error::None;
    }

    void __ISRStepRead()
    {
        // sample multiple times to stabilize reading
        constexpr int NB_SAMPLES = 9;
        Value raw_samples[NB_SAMPLES] = { 0 };
        for (int i = 0; i < NB_SAMPLES; i++)
        {
            Value raw_value;
            if (adc_oneshot_read(adc_handle, ADC_CHANNEL_1, &raw_value) != ESP_OK)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to read ADC value");
                // TODO : should we continue or break here?
            }
            raw_samples[i] = raw_value;
        }

        std::sort(raw_samples, raw_samples + NB_SAMPLES);
        Value raw_value_median = raw_samples[NB_SAMPLES / 2];

        // map raw value to mV
        adc_cali_raw_to_voltage(cali_handle, raw_value_median, (int*)&voltages_buffer[update_current_channel]);

        // select next index (lets time to settle before next update)
        update_current_channel = (update_current_channel + 1) % CHANNEL_COUNT;
        select(update_current_channel);
    }
}