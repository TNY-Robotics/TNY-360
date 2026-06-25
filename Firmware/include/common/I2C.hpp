#pragma once
#include <driver/i2c_master.h>
#include "common/utils.hpp"

namespace I2C
{
    // I2C master bus handle, used by critical modules (needing higher priority)
    extern i2c_master_bus_handle_t handle_primary;

    // I2C master bus handle, used by less critical modules (such as screen)
    extern i2c_master_bus_handle_t handle_secondary;

    /**
     * @brief Initializes the I2C module.
     * @return Error code indicating success or failure.
     */
    Status Init();

    /**
     * @brief Deinitializes the I2C module.
     * @return Error code indicating success or failure.
     */
    Status Deinit();

    Status ProbeAddress(i2c_master_bus_handle_t handle, uint8_t address);

    Status WriteRegisters(i2c_master_bus_handle_t handle, uint8_t address, uint8_t reg_address, const uint8_t* data, size_t length);

    Status ReadRegisters(i2c_master_bus_handle_t handle, uint8_t address, uint8_t reg_address, uint8_t* data, size_t length);
}