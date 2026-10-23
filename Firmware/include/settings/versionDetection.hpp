#pragma once
#include "common/utils.hpp"
#include "common/I2C.hpp"
#include "settings/Settings.hpp"
#include "settings/V1.hpp"
#include "settings/V2.hpp"
#include <driver/i2c_master.h>

namespace Settings
{
    bool newI2CDevice(gpio_num_t sda, gpio_num_t scl, i2c_master_bus_handle_t& handle)
    {
        i2c_master_bus_config_t config = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = sda,
            .scl_io_num = scl,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0, // no async transactions
            .flags = {
                .enable_internal_pullup = false,
                .allow_pd = false
            }
        };

        if (esp_err_t err = i2c_new_master_bus(&config, &handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to initialize I2C bus with error: 0x%0x", err);
            return false;
        }

        return true;
    }

    bool deleteI2CDevice(i2c_master_bus_handle_t handle)
    {
        if (esp_err_t err = i2c_del_master_bus(handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to delete I2C bus with error: 0x%0x", err);
            return false;
        }
        return true;
    }

    bool pingI2CDevice(i2c_master_bus_handle_t handle, uint8_t address)
    {
        esp_err_t err = i2c_master_probe(handle, address, 100);
        return (err == ESP_OK);
    }

    Status detectRobotVersion(RobotVersion& version)
    {
        bool wasI2CInitialized = I2C::IsInitialized();
        if (wasI2CInitialized)
        {
            LOG_DEBUG(TAG, "I2C already initialized, deinitializing for version detection");
            if (Status err = I2C::Deinit(); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Failed to deinitialize I2C for version detection, Error : %s", StatusToString(err));
                return err;
            }
        }

        // Detection is simple :
        // - try to ping the screen using V1 config.
        // - If it works, it's V1, if not, try V2 config.
        // - If it works, it's V2, if not, it's unknown.

        constexpr uint8_t SCREEN_I2C_ADDRESS = 0x3C;

        LOG_SCOPE(TAG, "Detecting robot version...");

        // First, try V1
        {
            LOG_SCOPE(TAG, "Trying V1 config...");
            i2c_master_bus_handle_t v1Handle;
            gpio_num_t sda = static_cast<gpio_num_t>(CONFIG_V1.i2c.secondary.gpio_sda);
            gpio_num_t scl = static_cast<gpio_num_t>(CONFIG_V1.i2c.secondary.gpio_scl);
            if (!newI2CDevice(sda, scl, v1Handle))
            {
                LOG_ERROR(TAG, "Failed to initialize V1 I2C bus");
                return Status::Failure;
            }

            if (pingI2CDevice(v1Handle, SCREEN_I2C_ADDRESS))
            {
                LOG_DEBUG(TAG, "Detected robot version: V1");
                deleteI2CDevice(v1Handle);
                version = RobotVersion::V1;
                goto cleanup;
            }

            deleteI2CDevice(v1Handle);
        }
        
        // Then, try V2
        {
            LOG_SCOPE(TAG, "Trying V2 config...");
            i2c_master_bus_handle_t v2Handle;
            gpio_num_t sda = static_cast<gpio_num_t>(CONFIG_V2.i2c.secondary.gpio_sda);
            gpio_num_t scl = static_cast<gpio_num_t>(CONFIG_V2.i2c.secondary.gpio_scl);
            if (!newI2CDevice(sda, scl, v2Handle))
            {
                LOG_ERROR(TAG, "Failed to initialize V2 I2C bus");
                return Status::Failure;
            }
            if (pingI2CDevice(v2Handle, SCREEN_I2C_ADDRESS))
            {
                LOG_DEBUG(TAG, "Detected robot version: V2");
                deleteI2CDevice(v2Handle);
                version = RobotVersion::V2;
                goto cleanup;
            }

            deleteI2CDevice(v2Handle);
        }

        // If we reach here, we couldn't detect the version
        LOG_ERROR(TAG, "Failed to detect robot version");
        version = RobotVersion::Unknown;
        goto cleanup;

    cleanup:
        if (wasI2CInitialized)
        {
            LOG_DEBUG(TAG, "I2C was already initialized, re-initializing after version detection");
            if (Status err = I2C::Init(); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Failed to re-initialize I2C for version detection, Error : %s", StatusToString(err));
                return err;
            }
        }
        return Status::Ok;
    }
}
