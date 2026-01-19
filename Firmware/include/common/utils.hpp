#pragma once
#include <cstdint>
#include <cstddef>

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
    OutOfBounds        // Used when an operation goes out of defined bounds
};

const char* ErrorToString(Error err);
