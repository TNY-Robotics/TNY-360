#pragma once
#include <cstdint>
#include "esp_timer.h"

struct PerfMonitor
{
    uint32_t iterations = 0;
    int64_t start_time = 0;
    int64_t total_time = 0;

    inline void start()
    {
        start_time = esp_timer_get_time();
    }

    inline void stop()
    {
        total_time += (esp_timer_get_time() - start_time);
        iterations++;
    }

    inline void reset()
    {
        total_time = 0;
        iterations = 0;
    }
    
    inline float get_avg_ms()
    {
        return (float)total_time / (iterations * 1000.0f);
    }
};