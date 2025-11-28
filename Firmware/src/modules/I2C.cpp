#include "modules/I2C.hpp"
#include "modules/Log.hpp"
#include "config.hpp"

namespace I2C
{
    Error Init()
    {
        i2c_master_bus_config_t config = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = I2C_SDA_GPIO,
            .scl_io_num = I2C_SCL_GPIO,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .flags = {
                .enable_internal_pullup = true,
                .allow_pd = false
            }
        };
        
        if (i2c_new_master_bus(&config, &handle) != ESP_OK)
        {
            handle = nullptr;
            Log::Add(Log::Level::Error, "I2C: Failed to initialize I2C bus");
            return Error::Unknown;
        }

        return Error::Ok;        
    }
}