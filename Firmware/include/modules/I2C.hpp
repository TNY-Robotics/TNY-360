#pragma once
#include <driver/i2c_master.h>
#include "utils.hpp"

namespace I2C
{
    i2c_master_bus_handle_t handle = nullptr;

    /**
     * @brief Initializes the I2C module.
     * @return Error code indicating success or failure.
     */
    Error Init();
}