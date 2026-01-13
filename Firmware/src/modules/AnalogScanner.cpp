#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include "modules/AnalogScanner.hpp"
#include "modules/Log.hpp"
#include "modules/NVS.hpp"
#include "config.hpp"
#include <vector>
#include <algorithm>

namespace AnalogScanner
{
    static bool initialized = false;
    static NVS::Handle *nvsHandle = nullptr;
    static adc_oneshot_unit_handle_t adc_handle = nullptr;
    static adc_cali_handle_t cali_handle = nullptr;
    // fsr, shoulder, elbow, leg
    // br, bl, fl, fr
    int min_raw_values[static_cast<uint8_t>(Id::Count)] = {
        0, 214, 214, 214,
        0, 214, 214, 214,
        0, 214, 214, 214,
        0, 214, 214, 214
    }; // raw min value, for calibration (mV)
    int max_raw_values[static_cast<uint8_t>(Id::Count)] = {
        0, 3084, 3084, 3084,
        0, 3084, 3084, 3084,
        0, 3084, 3084, 3084,
        0, 3084, 3084, 3084
    }; // raw max value, for calibration (mV)
    float min_calibrated_values[static_cast<uint8_t>(Id::Count)] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };  // calibrated min value, for calibration (real unit)
    float max_calibrated_values[static_cast<uint8_t>(Id::Count)] = {
        0.0f, 180.0f, 180.0f, 180.0f,
        0.0f, 180.0f, 180.0f, 180.0f,
        0.0f, 180.0f, 180.0f, 180.0f,
        0.0f, 180.0f, 180.0f, 180.0f
    };  // calibrated max value, for calibration (real unit)
    int last_raw_values[static_cast<uint8_t>(Id::Count)] = { 0 };             // last raw value read by the scan task (mV)
    float last_calibrated_values[static_cast<uint8_t>(Id::Count)] = { 0.0f }; // last calibrated value read by the scan task (real unit)

    static uint8_t update_current_channel = 0;
    
    Error select(uint8_t index)
    {
        if (gpio_set_level(SCANNER_SLCT_PIN1, (index & 0b0001) >> 0) != ESP_OK ||
            gpio_set_level(SCANNER_SLCT_PIN2, (index & 0b0010) >> 1) != ESP_OK ||
            gpio_set_level(SCANNER_SLCT_PIN3, (index & 0b0100) >> 2) != ESP_OK ||
            gpio_set_level(SCANNER_SLCT_PIN4, (index & 0b1000) >> 3) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "AnalogScanner: Failed to select index with GPIO");
            return Error::Unknown;
        }
        return Error::Ok;
    }

    Error Init()
    {
        if (initialized)
            return Error::Ok;

        // load nvs handle
        if (NVS::Open("analog_scanner", &nvsHandle) != Error::Ok)
        {
            Log::Add(Log::Level::Error, "AnalogScanner: Failed to open NVS namespace");
            return Error::Unknown;
        }

        // Setup select pins
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << SCANNER_SLCT_PIN1) | (1ULL << SCANNER_SLCT_PIN2) | (1ULL << SCANNER_SLCT_PIN3) | (1ULL << SCANNER_SLCT_PIN4);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        if (gpio_config(&io_conf) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "AnalogScanner: Failed to configure GPIO for select pins");
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
            Log::Add(Log::Level::Error, "AnalogScanner: Failed to create ADC oneshot handle");
            return Error::Unknown;
        }

        // Configure adc channel
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        if (adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_1, &config) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "AnalogScanner: Failed to configure ADC oneshot channel");
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
        return Error::Ok;
    }

    Error Calibrate(Id id, float raw_min, float raw_max, float real_min, float real_max)
    {
        uint8_t index = static_cast<uint8_t>(id);
        min_raw_values[index] = raw_min;
        max_raw_values[index] = raw_max;
        min_calibrated_values[index] = real_min;
        max_calibrated_values[index] = real_max;
        last_calibrated_values[index] = map<float>(last_raw_values[index], raw_min, raw_max, real_min, real_max);
        return Error::Ok;
    }

    Error SaveCalibration()
    {
        if (!nvsHandle)
            return Error::Unknown;

        if (nvsHandle->set("min_raw", min_raw_values, sizeof(min_raw_values)) != Error::Ok ||
            nvsHandle->set("max_raw", max_raw_values, sizeof(max_raw_values)) != Error::Ok ||
            nvsHandle->set("min_cal", min_calibrated_values, sizeof(min_calibrated_values)) != Error::Ok ||
            nvsHandle->set("max_cal", max_calibrated_values, sizeof(max_calibrated_values)) != Error::Ok)
        {
            Log::Add(Log::Level::Error, "AnalogScanner: Failed to save calibration data to NVS");
            return Error::Unknown;
        }

        return Error::Ok;
    }

    Error GetValue(Id id, float *outValue)
    {
        *outValue = last_calibrated_values[static_cast<uint8_t>(id)];
        return Error::Ok;
    }

    Error GetValues(float *outValues)
    {
        for (int i = 0; i < static_cast<int>(Id::Count); i++)
        {
            outValues[i] = last_calibrated_values[i];
        }
        return Error::Ok;
    }

    Error GetValues(const Id *ids, float *outValues, uint8_t count)
    {
        for (int i = 0; i < count; i++)
        {
            outValues[i] = last_calibrated_values[static_cast<uint8_t>(ids[i])];
        }
        return Error::Ok;
    }

    Error GetRawValue(Id id, int *outRawValue)
    {
        *outRawValue = last_raw_values[static_cast<uint8_t>(id)];
        return Error::Ok;
    }

    Error GetRawValues(int *outRawValues)
    {
        for (int i = 0; i < static_cast<int>(Id::Count); i++)
        {
            outRawValues[i] = last_raw_values[i];
        }
        return Error::Ok;
    }

    Error GetRawValues(const Id *ids, int *outRawValues, uint8_t count)
    {
        for (int i = 0; i < count; i++)
        {
            outRawValues[i] = last_raw_values[static_cast<uint8_t>(ids[i])];
        }
        return Error::Ok;
    }

    void _update_task(void *pvParams)
    {
        // sample multiple times to stabilize reading
        constexpr int NB_SAMPLES = 9;
        int raw_samples[NB_SAMPLES] = { 0 };
        for (int i = 0; i < NB_SAMPLES; i++)
        {
            int raw_value;
            if (adc_oneshot_read(adc_handle, ADC_CHANNEL_1, &raw_value) != ESP_OK)
            {
                Log::Add(Log::Level::Error, "AnalogScanner: Failed to read ADC value");
                // TODO : should we continue or break here?
            }
            raw_samples[i] = raw_value;
        }

        std::sort(raw_samples, raw_samples + NB_SAMPLES);
        int raw_value_median = raw_samples[NB_SAMPLES / 2];

        // map raw value to mV
        adc_cali_raw_to_voltage(cali_handle, raw_value_median, (int*)&last_raw_values[update_current_channel]);

        // map to calibrated value
        last_calibrated_values[update_current_channel] = map<float>(
            last_raw_values[update_current_channel], min_raw_values[update_current_channel],
            max_raw_values[update_current_channel], min_calibrated_values[update_current_channel],
            max_calibrated_values[update_current_channel]
        );
        
        // select next index (lets time to settle before next update)
        update_current_channel = (update_current_channel + 1) % static_cast<uint8_t>(Id::Count);
        select(update_current_channel);
    }
}