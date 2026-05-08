#pragma once
#include <cstdint>
#include <cstddef>
#include "esp_timer.h"
#include "common/ErrorCode.hpp"

enum class Error: uint8_t {
    None = 0,          // No error
    Unknown,           // Used for unspecified errors
    InvalidParameters, // Used when function parameters are invalid
    InvalidState,      // Used when the system is in an invalid state for the requested operation
    NotFound,          // Used when a requested item is not found
    NoMemory,          // Used when memory allocation fails or memory is insufficient
    HardwareFailure,   // Used when a hardware component fails
    SoftwareFailure,   // Used when a software component fails
    Unreachable,       // Used when a target is unreachable
    OutOfBounds,       // Used when an operation goes out of defined bounds
    OutOfMemory
};

// Helper macro to return early on error. Usage:
// RETURN_ERROR(SomeFunction());
#define RETURN_ERROR(x) if (Error err = (x); err != Error::None) return err;

const char* ErrorToString(Error err);

void ErrorHandle(ErrorStruct::ErrorStruct err);

struct PerfMonitor
{
    int64_t start_time = 0;
    int64_t total_time = 0;

    inline void start()
    {
        start_time = esp_timer_get_time();
    }

    inline void stop()
    {
        total_time += (esp_timer_get_time() - start_time);
    }

    inline void reset()
    {
        total_time = 0;
    }
    
    inline float get_avg_ms(int iterations)
    {
        return (float)total_time / (iterations * 1000.0f);
    }
};