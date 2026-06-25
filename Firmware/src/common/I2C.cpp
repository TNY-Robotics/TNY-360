#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "common/LED.hpp"
#include "common/I2C.Error.hpp"
#include <memory.h>

namespace I2C
{
    i2c_master_bus_handle_t handle_primary = nullptr;
    i2c_master_bus_handle_t handle_secondary = nullptr;

    constexpr const char* TAG = "I2C";

    static bool initialized = false;

    Status Init()
    {
        LOG_SCOPE(TAG, "I2C::Init");

        if (initialized) return Status::Ok;

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
        
        if (esp_err_t err = i2c_new_master_bus(&primary_config, &handle_primary); err != ESP_OK)
        {
            handle_primary = nullptr;
            LOG_ERROR(TAG, "Failed to initialize primary I2C bus");
            Error::RegisterErrorEvent(ErrorEventPrimaryInitFailed(err));
            return Status::Unknown;
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

        if (esp_err_t err = i2c_new_master_bus(&secondary_config, &handle_secondary); err != ESP_OK)
        {
            handle_secondary = nullptr;
            LOG_ERROR(TAG, "Failed to initialize secondary I2C bus");
            Error::RegisterErrorEvent(ErrorEventSecondaryInitFailed(err));
            return Status::Unknown;
        }

        initialized = true;
        return Status::Ok;
    }

    Status Deinit()
    {
        LOG_SCOPE(TAG, "I2C::Deinit");

        if (!initialized) return Status::Ok;

        if (handle_primary)
        {
            esp_err_t err = i2c_del_master_bus(handle_primary);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to delete primary I2C bus with error: 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventPrimaryDeinitFailed(err));
                return Status::Unknown;
            }
            handle_primary = nullptr;
        }

        if (handle_secondary)
        {
            esp_err_t err = i2c_del_master_bus(handle_secondary);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to delete secondary I2C bus with error: 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventSecondaryDeinitFailed(err));
                return Status::Unknown;
            }
            handle_secondary = nullptr;
        }

        initialized = false;
        return Status::Ok;
    }

    Status ProbeAddress(i2c_master_bus_handle_t handle, uint8_t address)
    {
        esp_err_t err = i2c_master_probe(handle, address, 100);
        if (err == ESP_OK)
        {
            return Status::Ok;
        }
        return Status::NotFound;
    }

    Status WriteRegisters(i2c_master_bus_handle_t handle, uint8_t address, uint8_t reg_address, const uint8_t* data, size_t length)
    {
        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = address;
        dev_cfg.scl_speed_hz = 400'000; // TODO : Maybe this should be configurable later?

        i2c_master_dev_handle_t dev_handle;
        if (esp_err_t err = i2c_master_bus_add_device(handle, &dev_cfg, &dev_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to add I2C device handle with error: 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventAddDeviceFailed(err));
            return Status::Failure;
        }

        uint8_t* buffer = (uint8_t*)malloc(length + 1);
        if (buffer == nullptr)
        {
            LOG_ERROR(TAG, "Failed to allocate memory for I2C write buffer");
            Error::RegisterErrorEvent(ErrorEventBufferAllocFailed());
            i2c_master_bus_rm_device(dev_handle);
            return Status::NoMemory;
        }
        
        buffer[0] = reg_address;
        memcpy(&buffer[1], data, length);

        if (esp_err_t err = i2c_master_transmit(dev_handle, buffer, length + 1, 1000); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to write to I2C device with error: 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventWriteRegistersFailed(err));
            i2c_master_bus_rm_device(dev_handle);
            free(buffer);
            return Status::Failure;
        }

        free(buffer);
        if (esp_err_t err = i2c_master_bus_rm_device(dev_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to remove I2C device handle with error: 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventRemoveDeviceFailed(err));
            return Status::Failure;
        }

        return Status::Ok;
    }

    Status ReadRegisters(i2c_master_bus_handle_t handle, uint8_t address, uint8_t reg_address, uint8_t* data, size_t length)
    {
        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = address;
        dev_cfg.scl_speed_hz = 400'000; // TODO : Maybe this should be configurable later?

        i2c_master_dev_handle_t dev_handle;
        if (esp_err_t err = i2c_master_bus_add_device(handle, &dev_cfg, &dev_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to add I2C device handle with error: 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventAddDeviceFailed(err));
            return Status::Failure;
        }

        if (esp_err_t err = i2c_master_transmit_receive(dev_handle, &reg_address, 1, data, length, 1000); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to read from I2C device with error: 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventReadRegistersFailed(err));
            i2c_master_bus_rm_device(dev_handle);
            return Status::Failure;
        }

        if (esp_err_t err = i2c_master_bus_rm_device(dev_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to remove I2C device handle with error: 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventRemoveDeviceFailed(err));
            return Status::Failure;
        }

        return Status::Ok;
    }
}