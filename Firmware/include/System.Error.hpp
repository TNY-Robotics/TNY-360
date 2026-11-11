#pragma once

#include "common/Error.hpp"
#include "esp_err.h"

namespace System
{
    enum class SubmoduleID : uint8_t
    {
        Boot = 0x01,
    };

    enum class CodeID : uint8_t
    {
        // Boot
        InitFailed = 0x01,
        StartFailed = 0x02,
    };

    /** <API_REF>
     * @error 0x01010104 InitFailed
     * @module System
     * @submodule Boot
     * @severity Critical
     * @desc Failed to initialize the robot.
     * @fix Check the other errors for more information.
     */
    class ErrorEventInitFailed : public Error::ErrorEventBuilder
    {
    public:
        ErrorEventInitFailed() :
            ErrorEventBuilder(Error::ModuleID::System, SubmoduleID::Boot, CodeID::InitFailed, Error::ErrorSeverity::Critical)
        {
        }
    };

    /** <API_REF>
     * @error 0x01010204 StartFailed
     * @module System
     * @submodule Boot
     * @severity Critical
     * @desc Failed to start the robot.
     * @fix Check the other errors for more information.
     */
    class ErrorEventStartFailed : public Error::ErrorEventBuilder
    {
    public:
        ErrorEventStartFailed() :
            ErrorEventBuilder(Error::ModuleID::System, SubmoduleID::Boot, CodeID::StartFailed, Error::ErrorSeverity::Critical)
        {
        }
    };
}