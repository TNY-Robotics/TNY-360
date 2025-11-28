#pragma once
#include <driver/i2c_master.h>
#include "utils.hpp"

namespace I2C
{
    // I2C master bus handle, used by critical modules (needing higher priority)
    static i2c_master_bus_handle_t handle_primary;

    // I2C master bus handle, used by less critical modules (such as screen)
    static i2c_master_bus_handle_t handle_secondary;

    /**
     * @brief Initializes the I2C module.
     * @return Error code indicating success or failure.
     */
    Error Init();
}