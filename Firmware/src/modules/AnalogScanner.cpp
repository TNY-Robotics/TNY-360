#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include "modules/AnalogScanner.hpp"
#include "modules/Log.hpp"
#include "modules/NVS.hpp"
#include "config.hpp"

namespace AnalogScanner
{
    bool initialized = false;
    NVS::Handle *nvsHandle = nullptr;
    adc_oneshot_unit_handle_t adc_handle = nullptr;

    int min_raw_values[static_cast<uint8_t>(Id::Count)] = { 0 };         // raw min value, for calibration
    int max_raw_values[static_cast<uint8_t>(Id::Count)] = { 0 };         // raw max value, for calibration
    float min_calibrated_values[static_cast<uint8_t>(Id::Count)] = { 0.0f };  // calibrated min value, for calibration
    float max_calibrated_values[static_cast<uint8_t>(Id::Count)] = { 0.0f };  // calibrated max value, for calibration
    int last_raw_values[static_cast<uint8_t>(Id::Count)] = { 0 };        // last raw value read by the scan task
    float last_calibrated_values[static_cast<uint8_t>(Id::Count)] = { 0.0f }; // last calibrated value read by the scan task
    
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

    void update_task(void *pvParameters)
    {
        TickType_t start_tick = xTaskGetTickCount();
        TickType_t end_tick = xTaskGetTickCount();
        while (true)
        {
            start_tick = xTaskGetTickCount();
            for (int i = 0; i < 16; i++)
            {
                select(i);
                vTaskDelay(pdMS_TO_TICKS(1));
                if (adc_oneshot_read(adc_handle, ADC_CHANNEL_1, &last_raw_values[i]) != ESP_OK)
                {
                    Log::Add(Log::Level::Error, "AnalogScanner: Failed to read ADC value");
                    // FIXME : should we continue or break here?
                }
                last_calibrated_values[i] = map<float>(
                    last_raw_values[i], min_raw_values[i],
                    max_raw_values[i], min_calibrated_values[i],
                    max_calibrated_values[i]
                );
            }
            end_tick = xTaskGetTickCount();
            vTaskDelay(pdMS_TO_TICKS(SCANNER_UPDT_INT_MS) - (end_tick - start_tick));
        }
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

        // launch update task
        if (xTaskCreate(update_task, "AnalogScanner::update_task", 2048, nullptr, 5, nullptr) != pdPASS)
        {
            Log::Add(Log::Level::Error, "AnalogScanner: Failed to create update task");
            return Error::Unknown;
        }

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
}