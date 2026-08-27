#include "common/utils.hpp"

const char* StatusToString(Status status)
{
    switch (status)
    {
        case Status::Ok: return "Ok";
        case Status::InvalidParameters: return "InvalidParameters";
        case Status::InvalidState: return "InvalidState";
        case Status::NotFound: return "NotFound";
        case Status::NoMemory: return "NoMemory";
        case Status::OutOfBounds: return "OutOfBounds";
        case Status::Failure: return "Failure";
        case Status::Unknown: return "Unknown";
        default: return "---UnknownStatus---";
    }
}