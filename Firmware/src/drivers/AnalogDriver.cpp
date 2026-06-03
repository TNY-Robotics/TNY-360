#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "soc/gpio_reg.h" // for the direct register manipulation in select function
#include <esp_adc/adc_oneshot.h>
#include "drivers/AnalogDriver.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "common/analysis/ArrayStats.hpp"
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
        Error select(Channel channel)
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
                return Error::Unknown;
            }
            return Error::None;
        }

        Error read(Value& outVoltage)
        {
            int raw_value;
            if (adc_oneshot_read(adc_handle, ADC_CHANNEL_1, &raw_value) != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to read ADC value");
                return Error::HardwareFailure;
            }
            int mv_value;
            adc_cali_raw_to_voltage(cali_handle, raw_value, &mv_value);
            outVoltage = static_cast<Value>(mv_value) / 1000.f; // convert to Volt
            return Error::None;
        }

        Error read_subsampled(Value& outVoltage, uint16_t nb_subsamples)
        {
            Value samples[nb_subsamples];
            for (uint16_t i = 0; i < nb_subsamples; i++)
            {
                RETURN_ERROR(read(samples[i]));
            }
            outVoltage = ArrayStats::GetStats(samples, nb_subsamples).mean;
            return Error::None;
        }
    }

    Error Init()
    {
        LOG_SCOPE(TAG, "AnalogDriver::Init");

        if (initialized) return Error::None;

        // Setup select pins
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << SCANNER_SLCT_PIN1) | (1ULL << SCANNER_SLCT_PIN2) | (1ULL << SCANNER_SLCT_PIN3) | (1ULL << SCANNER_SLCT_PIN4);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        if (gpio_config(&io_conf) != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to configure GPIO for select pins");
            ErrorHandle(ErrorStruct::ReaderInitFailed);
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
            LOG_ERROR(TAG, "Failed to create ADC oneshot handle");
            ErrorHandle(ErrorStruct::ReaderInitFailed);
            return Error::Unknown;
        }

        // Configure adc channel
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        if (adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_1, &config) != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to configure ADC oneshot channel");
            ErrorHandle(ErrorStruct::ReaderInitFailed);
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
        internal::select(cur_channel); // 0

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

    Error GetVoltage(Channel id, Value& outVoltage)
    {
        if (id >= CHANNEL_COUNT)
        {
            LOG_ERROR(TAG, "GetVoltage: Invalid channel id %d", id);
            return Error::InvalidParameters;
        }

        outVoltage = voltages_buffer[id];
        return Error::None;
    }

    Error GetVoltages(Value* outVoltages)
    {
        for (size_t i = 0; i < static_cast<size_t>(CHANNEL_COUNT); i++)
        {
            outVoltages[i] = voltages_buffer[i];
        } // maybe memcpy?
        return Error::None;
    }

    Error GetVoltages(const Channel* ids, Value* outVoltages, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            Channel id = ids[i];
            if (id >= CHANNEL_COUNT)
            {
                LOG_ERROR(TAG, "GetVoltages: Invalid channel id %d", id);
                return Error::InvalidParameters;
            }
            outVoltages[i] = voltages_buffer[id];
        }
        return Error::None;
    }

    Error ReadAllChannels()
    {
        float val;
        for (int i = 0; i < CHANNEL_COUNT; i++)
        {
            RETURN_ERROR(internal::select(i));
            esp_rom_delay_us(10); // Wait for signal to stabilize after switching
            RETURN_ERROR(internal::read(val));
            voltages_buffer[i] += ANALOG_EMA_ALPHA * (val - voltages_buffer[i]);
        }
        return Error::None;
    }
}