#pragma once
#include <driver/i2c_master.h>
#include "utils.hpp"

namespace I2C
{
    // I2C master bus handle, available to other modules for their own use
    static i2c_master_bus_handle_t handle;

    /**
     * @brief Initializes the I2C module.
     * @return Error code indicating success or failure.
     */
    Error Init();
}