#include "common/utils.hpp"

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
        default: return "???";
    }
}