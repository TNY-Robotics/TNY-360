#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "common/LED.hpp"
#include <memory.h>

namespace I2C
{
    i2c_master_bus_handle_t handle_primary = nullptr;
    i2c_master_bus_handle_t handle_secondary = nullptr;

    constexpr const char* TAG = "I2C";

    static bool initialized = false;

    Error Init()
    {
        LOG_SCOPE(TAG, "I2C::Init");

        if (initialized) return Error::None;

        // First setup the primary I2C bus
        i2c_master_bus_config_t primary_config = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = I2C_PRIMARY_SDA_GPIO_NUM,
            .scl_io_num = I2C_PRIMARY_SCL_GPIO_NUM,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0, // no async transactions
            .flags = {
                .enable_internal_pullup = false,
                .allow_pd = false
            }
        };
        
        if (i2c_new_master_bus(&primary_config, &handle_primary) != ESP_OK)
        {
            handle_primary = nullptr;
            LOG_ERROR(TAG, "Failed to initialize primary I2C bus");
            ErrorHandle(ErrorStruct::I2cBusPrimaryInitFailed);
            return Error::Unknown;
        }

        // Then setup the secondary I2C bus
        i2c_master_bus_config_t secondary_config = {
            .i2c_port = I2C_NUM_1,
            .sda_io_num = I2C_SECONDARY_SDA_GPIO_NUM,
            .scl_io_num = I2C_SECONDARY_SCL_GPIO_NUM,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0, // no async transactions
            .flags = {
                .enable_internal_pullup = false,
                .allow_pd = false
            }
        };

        if (i2c_new_master_bus(&secondary_config, &handle_secondary) != ESP_OK)
        {
            handle_secondary = nullptr;
            LOG_ERROR(TAG, "Failed to initialize secondary I2C bus");
            ErrorHandle(ErrorStruct::I2cBusSecondaryInitFailed);
            return Error::Unknown;
        }

        initialized = true;
        return Error::None;
    }

    Error ProbeAddress(i2c_master_bus_handle_t handle, uint8_t address)
    {
        esp_err_t err = i2c_master_probe(handle, address, 100);
        if (err == ESP_OK)
        {
            return Error::None;
        }
        return Error::NotFound;
    }

    Error WriteRegisters(i2c_master_bus_handle_t handle, uint8_t address, uint8_t reg_address, const uint8_t* data, size_t length)
    {
        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = address;
        dev_cfg.scl_speed_hz = 400'000; // TODO : Maybe this should be configurable later?

        i2c_master_dev_handle_t dev_handle;
        if (i2c_master_bus_add_device(handle, &dev_cfg, &dev_handle) != ESP_OK) {
            return Error::SoftwareFailure;
        }

        uint8_t* buffer = (uint8_t*)malloc(length + 1);
        if (buffer == nullptr) {
            i2c_master_bus_rm_device(dev_handle);
            return Error::OutOfMemory;
        }
        
        buffer[0] = reg_address;
        memcpy(&buffer[1], data, length);

        esp_err_t err = i2c_master_transmit(dev_handle, buffer, length + 1, 1000); // Timeout 1000ms

        free(buffer);
        i2c_master_bus_rm_device(dev_handle);

        return (err == ESP_OK) ? Error::None : Error::SoftwareFailure;
    }

    Error ReadRegisters(i2c_master_bus_handle_t handle, uint8_t address, uint8_t reg_address, uint8_t* data, size_t length)
    {
        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = address;
        dev_cfg.scl_speed_hz = 400'000; // TODO : Maybe this should be configurable later?

        i2c_master_dev_handle_t dev_handle;
        if (i2c_master_bus_add_device(handle, &dev_cfg, &dev_handle) != ESP_OK) {
            return Error::SoftwareFailure;
        }

        esp_err_t err = i2c_master_transmit_receive(
            dev_handle, 
            &reg_address, 1,
            data, length,
            1000
        );

        i2c_master_bus_rm_device(dev_handle);

        return (err == ESP_OK) ? Error::None : Error::SoftwareFailure;
    }
}