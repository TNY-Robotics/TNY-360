#pragma once
#include "common/utils.hpp"

namespace SysStats
{
    typedef struct CPUUsage
    {
        float core0; // in percentage, 0 -> 100
        float core1; // in percentage, 0 -> 100
    } CPUUsage;

    typedef struct RAMUsage {
        uint32_t internal_total; // in bytes
        uint32_t internal_used; // in bytes
        uint32_t psram_total; // in bytes
        uint32_t psram_used; // in bytes
    } RAMUsage;

    /**
     * @brief Get the current CPU usage for both cores.
     * @return CPUUsage struct containing usage percentages (0-100) for core0 and core1. 
     */
    CPUUsage GetCPUUsage();

    /**
     * @brief Get the current RAM usage for both internal and PSRAM.
     * @return RAMUsage struct containing total and used memory in bytes for internal and PSRAM.
     */
    RAMUsage GetRAMUsage();

    /**
     * @brief Get the current temperature of the system.
     * @return Temperature in degrees Celsius.
     */
    float GetTemperature();
};
