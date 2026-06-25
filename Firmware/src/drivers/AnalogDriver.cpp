#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "soc/gpio_reg.h" // for the direct register manipulation in select function
#include <esp_adc/adc_oneshot.h>
#include "drivers/AnalogDriver.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "common/analysis/ArrayStats.hpp"
#include "drivers/AnalogDriver.Error.hpp"
#include <vector>
#include <algorithm>

namespace AnalogDriver
{
    constexpr const char* TAG = "AnalogDriver";

    static bool initialized = false;
    static adc_oneshot_unit_handle_t adc_handle = nullptr;
    static adc_cali_handle_t cali_handle = nullptr;

    static Value voltages_buffer[static_cast<size_t>(CHANNEL_COUNT)] = { 0 };

    static Channel cur_channel = 0;
    
    namespace internal
    {
        Status select(Channel channel)
        {
            // NOTE : Really optimised version, we cannot modify select pins anymore
            // // Mask to keep only the 4 LSB (0-15) of the channel, as we have only 4 select lines for the multiplexer
            // uint32_t channel_bits = (channel & 0x0F); 

            // // Shift to align with GPIO pins
            // uint32_t set_mask = channel_bits << 7; 
            
            // // Inverse bits to find the ones to set to LOW
            // // and also shift to align with GPIO pins
            // uint32_t clear_mask = (~channel_bits & 0x0F) << 7; 

            // // Directly write to ESP32-S3 GPIO memory (Bank 1 = GPIO 32 to 53)
            // REG_WRITE(GPIO_OUT1_W1TS_REG, set_mask);  // Set (W1TS = Write 1 To Set)
            // REG_WRITE(GPIO_OUT1_W1TC_REG, clear_mask); // Clear (W1TC = Write 1 To Clear)

            // Old version using gpio_set_level, much slower due to multiple function calls and checks
            if (gpio_set_level(SCANNER_SLCT_PIN1, (channel & 0b0001) >> 0) != ESP_OK ||
                gpio_set_level(SCANNER_SLCT_PIN2, (channel & 0b0010) >> 1) != ESP_OK ||
                gpio_set_level(SCANNER_SLCT_PIN3, (channel & 0b0100) >> 2) != ESP_OK ||
                gpio_set_level(SCANNER_SLCT_PIN4, (channel & 0b1000) >> 3) != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to select index with GPIO");
                Error::RegisterErrorEvent(ErrorEventGPIOSelectFailed(ESP_FAIL));
                return Status::Unknown;
            }
            return Status::Ok;
        }

        Status read(Value& outVoltage)
        {
            int raw_value;
            if (esp_err_t err = adc_oneshot_read(adc_handle, ADC_CHANNEL_1, &raw_value); err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to read ADC value with error: 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventADCReadFailed(err));
                return Status::Failure;
            }
            int mv_value;
            if (esp_err_t err = adc_cali_raw_to_voltage(cali_handle, raw_value, &mv_value); err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to convert ADC raw value to voltage with error: 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventConversionFailed(err));
                return Status::Failure;
            }
            outVoltage = static_cast<Value>(mv_value) / 1000.f; // convert to Volt
            return Status::Ok;
        }

        Status read_subsampled(Value& outVoltage, uint16_t nb_subsamples)
        {
            Value samples[nb_subsamples];
            for (uint16_t i = 0; i < nb_subsamples; i++)
            {
                RETURN_ON_ERROR(read(samples[i]));
            }
            outVoltage = ArrayStats::GetStats(samples, nb_subsamples).mean;
            return Status::Ok;
        }
    }

    Status Init()
    {
        LOG_SCOPE(TAG, "AnalogDriver::Init");

        if (initialized) return Status::Ok;

        // Setup select pins
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << SCANNER_SLCT_PIN1) | (1ULL << SCANNER_SLCT_PIN2) | (1ULL << SCANNER_SLCT_PIN3) | (1ULL << SCANNER_SLCT_PIN4);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        if (esp_err_t err = gpio_config(&io_conf); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to configure GPIO for select pins");
            Error::RegisterErrorEvent(ErrorEventGPIOConfigFailed(err));
            return Status::Unknown;
        }

        // Create adc oneshot handle
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
            .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
        if (esp_err_t err = adc_oneshot_new_unit(&init_config, &adc_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to create ADC oneshot handle");
            Error::RegisterErrorEvent(ErrorEventOneshotInitFailed(err));
            return Status::Unknown;
        }

        // Configure adc channel
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        if (esp_err_t err = adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_1, &config); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to configure ADC oneshot channel");
            Error::RegisterErrorEvent(ErrorEventOneshotConfigFailed(err));
            return Status::Unknown;
        }

        // configure calibration handle
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .chan = ADC_CHANNEL_1,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        if (esp_err_t err = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to create ADC calibration handle");
            Error::RegisterErrorEvent(ErrorEventCalibrationInitFailed(err));
            return Status::Unknown;
        }

        // select initial channel
        RETURN_ON_ERROR(internal::select(cur_channel));

        initialized = true;
        return Status::Ok;
    }

    Status Deinit()
    {
        if (!initialized)
            return Status::Ok;

        if (esp_err_t err = adc_oneshot_del_unit(adc_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to delete ADC oneshot handle");
            Error::RegisterErrorEvent(ErrorEventOneshotDeleteFailed(err));
            return Status::Failure;
        }
        adc_handle = nullptr;

        if (esp_err_t err = adc_cali_delete_scheme_curve_fitting(cali_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to delete Curve Fitting calibration handle");
            Error::RegisterErrorEvent(ErrorEventCalibrationDeleteFailed(err));
            return Status::Failure;
        }
        cali_handle = nullptr;

        initialized = false;
        return Status::Ok;
    }

    Status GetVoltage(Channel id, Value& outVoltage)
    {
        if (id >= CHANNEL_COUNT)
        {
            LOG_ERROR(TAG, "GetVoltage: Invalid channel id %d", id);
            return Status::InvalidParameters;
        }

        outVoltage = voltages_buffer[id];
        return Status::Ok;
    }

    Status GetVoltages(Value* outVoltages)
    {
        for (size_t i = 0; i < static_cast<size_t>(CHANNEL_COUNT); i++)
        {
            outVoltages[i] = voltages_buffer[i];
        } // maybe memcpy?
        return Status::Ok;
    }

    Status GetVoltages(const Channel* ids, Value* outVoltages, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            Channel id = ids[i];
            if (id >= CHANNEL_COUNT)
            {
                LOG_ERROR(TAG, "GetVoltages: Invalid channel id %d", id);
                return Status::InvalidParameters;
            }
            outVoltages[i] = voltages_buffer[id];
        }
        return Status::Ok;
    }

    Status ReadAllChannels()
    {
        float val;
        for (int i = 0; i < CHANNEL_COUNT; i++)
        {
            RETURN_ON_ERROR(internal::select(i));
            esp_rom_delay_us(10); // Wait for signal to stabilize after switching
            RETURN_ON_ERROR(internal::read(val));
            voltages_buffer[i] += ANALOG_EMA_ALPHA * (val - voltages_buffer[i]);
        }
        return Status::Ok;
    }
}