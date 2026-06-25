#include "drivers/PowerDriver.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "drivers/PowerDriver.Error.hpp"
#include "ina219.h"

namespace PowerDriver
{
    bool initialized = false;
    static ina219_handle_t ina_handle;
    static Data power_data;

    namespace internal
    {
        Status read_voltage(Value& voltage_v)
        {
            if (esp_err_t err = ina219_get_bus_voltage_V(ina_handle, &voltage_v); err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to read voltage data from INA219 with error 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventReadVoltageFailed(err));
                return Status::Failure;
            }
            return Status::Ok;
        }

        Status read_current(Value& current_a)
        {
            float value;
            if (esp_err_t err = ina219_get_current_mA(ina_handle, &value); err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to read current data from INA219 with error 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventReadCurrentFailed(err));
                return Status::Failure;
            }
            current_a = value / 1000.f; // from mA to A
            return Status::Ok;
        }

        Status read_power(Value& power_w)
        {
            float value;
            if (esp_err_t err = ina219_get_power_mW(ina_handle, &value); err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to read power data from INA219 with error 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventReadPowerFailed(err));
                return Status::Failure;
            }
            power_w = value / 1000.f; // from mW to W
            return Status::Ok;
        }
    }

    Status Init()
    {
        LOG_SCOPE(TAG, "PowerDriver::Init");

        if (initialized) return Status::Ok;
        
        if (Status err = I2C::Init(); err != Status::Ok)
        {
            Error::RegisterErrorEvent(ErrorEventI2CInitFailed());
            return err;
        }

        ina219_info_t ina_info = INA219_DEFAULT_INFO();
        ina_info.clock_speed = 400'000; // Fast-mode

        if (esp_err_t err = ina219_create(I2C::handle_secondary, ina_info, &ina_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to create INA219 handle");
            Error::RegisterErrorEvent(ErrorEventCreateFailed(err));
            return Status::Failure;
        }

        ina219_config_t ina_config = {
            .bus_voltage_range = INA219_BUS_VOLTAGE_RANGE_16V, // 16V range
            .gain = INA219_GAIN_8_320MV, // max shunt voltage 320mV (for 16A with 2mOhm shunt)
            .badc_res = INA219_ADC_12BIT_16SAMP, // 12-bit, 16 samples averaged (smoother but still quick)
            .sadc_res = INA219_ADC_12BIT_16SAMP, // same
            .mode = INA219_MODE_SHUNT_BUS_CONT, // continuous shunt and bus voltage measurement mode
            .shunt_resistor_ohms = 0.002f, // 2 milliohms
            .max_expected_current_A = 16.0f // 16A max
        };

        if (esp_err_t err = ina219_config(&ina_handle, ina_config); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to configure INA219");
            Error::RegisterErrorEvent(ErrorEventConfigFailed(err));
            return Status::Failure;
        }

        if (esp_err_t err = ina219_wake_up(ina_handle, INA219_MODE_SHUNT_BUS_CONT); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to wake up INA219");
            Error::RegisterErrorEvent(ErrorEventWakeUpFailed(err));
            return Status::Failure;
        }

        initialized = true;
        return Status::Ok;
    }

    Status Deinit()
    {
        if (esp_err_t err = ina219_delete(ina_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to delete INA219 handle");
            Error::RegisterErrorEvent(ErrorEventDeleteFailed(err));
            return Status::Failure;
        }
        initialized = false;
        return Status::Ok;
    }

    Status ReadData()
    {
        if (Status err = internal::read_voltage(power_data.voltage_v); err != Status::Ok)
        {
            return err;
        }
        if (Status err = internal::read_current(power_data.current_a); err != Status::Ok)
        {
            return err;
        }
        if (Status err = internal::read_power(power_data.power_w); err != Status::Ok)
        {
            return err;
        }
        return Status::Ok;
    }

    Data& GetData()
    {
        return power_data;
    }
}
