#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"

namespace I2C
{
    i2c_master_bus_handle_t handle_primary = nullptr;
    i2c_master_bus_handle_t handle_secondary = nullptr;

    constexpr const char* TAG = "I2C";

    static bool initialized = false;

    Error Init()
    {
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
                .enable_internal_pullup = true,
                .allow_pd = false
            }
        };
        
        if (i2c_new_master_bus(&primary_config, &handle_primary) != ESP_OK)
        {
            handle_primary = nullptr;
            Log::Add(Log::Level::Error, TAG, "I2C: Failed to initialize primary I2C bus");
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
                .enable_internal_pullup = true,
                .allow_pd = false
            }
        };

        if (i2c_new_master_bus(&secondary_config, &handle_secondary) != ESP_OK)
        {
            handle_secondary = nullptr;
            Log::Add(Log::Level::Error, TAG, "I2C: Failed to initialize secondary I2C bus");
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
}