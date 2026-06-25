#pragma once
#include "common/Error.hpp"
#include "esp_err.h"

enum class SubmoduleID : uint8_t
{
    Initialization = 0x01,
    Runtime = 0x02,
    Cleanup = 0x03,
};

enum class CodeID : uint8_t
{
    // Initialization
    I2CInitFailed = 0x01,
    CreateFailed = 0x02,
    ConfigFailed = 0x03,
    WakeUpFailed = 0x04,

    // Runtime
    ReadVoltageFailed = 0x01,
    ReadCurrentFailed = 0x02,
    ReadPowerFailed = 0x03,

    // Cleanup
    DeleteFailed = 0x01,
};

/** <API_REF>
 * @error 0x0B010101 I2CInitFailed
 * @desc Indicates that the I2C initialization failed during initialization.
 * @fix Check that the I2C bus is properly configured and available for use by the Power driver.
 */
class ErrorEventI2CInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventI2CInitFailed() :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Initialization, CodeID::I2CInitFailed, Error::ErrorLevel::Trace)
    {}
};

/** <API_REF>
 * @error 0x0B010201 CreateFailed
 * @desc Indicates that the INA219 handle creation failed during initialization.
 * @fix Check that the INA219 device is properly connected and available on the I2C bus.
 * @payload err uint32 The error code returned by the INA219 handle creation function.
 */
class ErrorEventCreateFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCreateFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Initialization, CodeID::CreateFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0B010301 ConfigFailed
 * @desc Indicates that the INA219 configuration failed during initialization.
 * @fix Check that the INA219 device is properly connected and available on the I2C bus.
 * @payload err uint32 The error code returned by the INA219 configuration function.
 */
class ErrorEventConfigFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventConfigFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Initialization, CodeID::ConfigFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0B010401 WakeUpFailed
 * @desc Indicates that the INA219 wake-up failed during initialization.
 * @fix Check that the INA219 device is properly connected and available on the I2C bus.
 * @payload err uint32 The error code returned by the INA219 wake-up function.
 */
class ErrorEventWakeUpFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventWakeUpFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Initialization, CodeID::WakeUpFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0B020101 ReadVoltageFailed
 * @desc Indicates that reading the voltage from the INA219 failed during runtime.
 * @fix Check that the INA219 device is properly connected and available on the I2C bus.
 * @payload err uint32 The error code returned by the INA219 read voltage function.
 */
class ErrorEventReadVoltageFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventReadVoltageFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Runtime, CodeID::ReadVoltageFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0B020201 ReadCurrentFailed
 * @desc Indicates that reading the current from the INA219 failed during runtime.
 * @fix Check that the INA219 device is properly connected and available on the I2C bus.
 * @payload err uint32 The error code returned by the INA219 read current function.
 */
class ErrorEventReadCurrentFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventReadCurrentFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Runtime, CodeID::ReadCurrentFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0B020301 ReadPowerFailed
 * @desc Indicates that reading the power from the INA219 failed during runtime.
 * @fix Check that the INA219 device is properly connected and available on the I2C bus.
 * @payload err uint32 The error code returned by the INA219 read power function.
 */
class ErrorEventReadPowerFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventReadPowerFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Runtime, CodeID::ReadPowerFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0B030101 DeleteFailed
 * @desc Indicates that deleting the INA219 handle failed during cleanup.
 * @fix Check that the INA219 device is properly connected and available on the I2C bus.
 * @payload err uint32 The error code returned by the INA219 delete function.
 */
class ErrorEventDeleteFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventDeleteFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Power, SubmoduleID::Cleanup, CodeID::DeleteFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};