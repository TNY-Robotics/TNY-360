#include "common/SysStats.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_heap_caps.h"
#include "driver/temperature_sensor.h"
#include <string.h>

namespace SysStats
{
    //******// CPU Usage variables
    static uint32_t last_total_runtime = 0;
    static uint32_t last_idle0_time = 0;
    static uint32_t last_idle1_time = 0;

    CPUUsage GetCPUUsage()
    {
        CPUUsage current_usage = {0.0f, 0.0f};
        uint32_t task_count = uxTaskGetNumberOfTasks();
        TaskStatus_t *status_array = (TaskStatus_t *)pvPortMalloc(task_count * sizeof(TaskStatus_t));

        if (status_array != NULL) {
            uint32_t total_runtime;
            task_count = uxTaskGetSystemState(status_array, task_count, &total_runtime);

            if (total_runtime > 0 && total_runtime != last_total_runtime) {
                uint32_t idle0_time = 0;
                uint32_t idle1_time = 0;

                // Find the idle task times
                for (int i = 0; i < task_count; i++) {
                    if (strcmp(status_array[i].pcTaskName, "IDLE0") == 0) {
                        idle0_time = status_array[i].ulRunTimeCounter;
                    } else if (strcmp(status_array[i].pcTaskName, "IDLE1") == 0) {
                        idle1_time = status_array[i].ulRunTimeCounter;
                    }
                }

                // Delta time calculations
                float delta_total = (float)(total_runtime - last_total_runtime);
                float delta_idle0 = (float)(idle0_time - last_idle0_time);
                float delta_idle1 = (float)(idle1_time - last_idle1_time);

                // Usage = 100 * (1 - idle_time / total_time)
                // Clamp to [0, 100] just in case of anomalies
                current_usage.core0 = (1.0f - (delta_idle0 / delta_total)) * 100.0f;
                current_usage.core1 = (1.0f - (delta_idle1 / delta_total)) * 100.0f;

                // Save for next calculation
                last_total_runtime = total_runtime;
                last_idle0_time = idle0_time;
                last_idle1_time = idle1_time;
            }
            vPortFree(status_array);
        }
        return current_usage;
    }

    RAMUsage GetRAMUsage()
    {
        RAMUsage stats;

        // INTERNAL RAM
        stats.internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        uint32_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        stats.internal_used = stats.internal_total - internal_free;

        // PSRAM (SPIRAM)
        stats.psram_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        uint32_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        stats.psram_used = stats.psram_total - psram_free;

        return stats;
    }

    float GetTemperature()
    {
        static temperature_sensor_handle_t temp_sensor = NULL;
        static bool is_initialized = false;

        if (!is_initialized) {
            temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
            if (temperature_sensor_install(&temp_sensor_config, &temp_sensor) == ESP_OK) {
                temperature_sensor_enable(temp_sensor);
                is_initialized = true;
            } else {
                return 0.0f; // Init error, return 0 as a safe default
            }
        }

        float tsens_out;
        temperature_sensor_get_celsius(temp_sensor, &tsens_out);
        return tsens_out;
    }
}