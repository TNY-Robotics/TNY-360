#pragma once
#include "common/utils.hpp"

namespace SysStats
{
    /** <API_REF>
     * @type CPUUsage
     * @desc Statistics about the CPU usage of the robot.
     * @field core0 float32 CPU usage percentage of core 0 (0-100).
     * @field core1 float32 CPU usage percentage of core 1 (0-100).
     */
    typedef struct CPUUsage
    {
        float core0;
        float core1;
    } CPUUsage;

    /** <API_REF>
     * @type RAMUsage
     * @desc Statistics about the robot's memory usage.
     * @field internal_total uint32 Total number of bytes in internal RAM.
     * @field internal_used uint32 Number of bytes used in internal RAM.
     * @field psram_total uint32 Total number of bytes in PSRAM.
     * @field psram_used uint32 Number of bytes used in PSRAM.
     */
    typedef struct RAMUsage {
        uint32_t internal_total;
        uint32_t internal_used;
        uint32_t psram_total;
        uint32_t psram_used;
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
