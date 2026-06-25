#pragma once
#include <cstdint>
#include <cstddef>
#include "common/Error.hpp"

// Helper macro to return early on error. Usage:
// RETURN_ON_ERROR(SomeFunction());
#define RETURN_ON_ERROR(x) if (Status status = (x); status != Status::Ok) return status;

enum class Status: uint8_t
{
    Ok = 0,            // No error
    InvalidParameters, // Used when function parameters are invalid
    InvalidState,      // Used when the system is in an invalid state for the requested operation
    NotFound,          // Used when a requested item is not found
    NoMemory,          // Used when memory allocation fails or memory is insufficient
    OutOfBounds,       // Used when an operation attempts to access data outside of valid bounds
    Failure,           // Used for general failures (it should have been ok, but something went wrong)
    Unknown,           // Used for unspecified errors
};