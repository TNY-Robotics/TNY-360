#include "common/utils.hpp"
#include "common/LED.hpp"
#include "ui/Menus.hpp"

const char* ErrorToString(Error err)
{
    switch (err)
    {
        case Error::None: return "None";
        case Error::Unknown: return "Unknown";
        case Error::InvalidParameters: return "InvalidParameters";
        case Error::InvalidState: return "InvalidState";
        case Error::NotFound: return "NotFound";
        case Error::NoMemory: return "NoMemory";
        case Error::HardwareFailure: return "HardwareFailure";
        case Error::SoftwareFailure: return "SoftwareFailure";
        case Error::Unreachable: return "Unreachable";
        case Error::OutOfBounds: return "OutOfBounds";
        case Error::OutOfMemory: return "OutOfMemory";
        default: return "???";
    }
}

void ErrorHandle(ErrorStruct::ErrorStruct err)
{
    // Display error code on LED
    LED::LoopErrorCode(err.code);

    // If display is available, show the detailed error message
    Menus::DisplayError(err);
}