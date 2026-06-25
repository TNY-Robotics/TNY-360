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
    PrimaryInitFailed = 0x01,
    SecondaryInitFailed = 0x02,

    // Runtime
    AddDeviceFailed = 0x01,
    RemoveDeviceFailed = 0x02,
    BufferAllocFailed = 0x03,
    WriteRegistersFailed = 0x04,
    ReadRegistersFailed = 0x05,

    // Cleanup
    PrimaryDeinitFailed = 0x01,
    SecondaryDeinitFailed = 0x02,
};

/** <API_REF>
 * @error 0x0F010101 ErrorEventPrimaryInitFailed
 * @desc The primary I2C bus initialization failed.
 * @fix Ensure that the I2C bus is properly connected and that the pins are correctly configured.
 * @payload err uint32 The error code returned by the I2C initialization function.
 */
class ErrorEventPrimaryInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventPrimaryInitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Initialization, CodeID::PrimaryInitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0F010201 ErrorEventSecondaryInitFailed
 * @desc The secondary I2C bus initialization failed.
 * @fix Ensure that the I2C bus is properly connected and that the pins are correctly configured.
 * @payload err uint32 The error code returned by the I2C initialization function.
 */
class ErrorEventSecondaryInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventSecondaryInitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Initialization, CodeID::SecondaryInitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0F030101 ErrorEventPrimaryDeinitFailed
 * @desc The primary I2C bus deinitialization failed.
 * @fix Ensure that the I2C bus is properly connected and that the pins are correctly configured.
 * @payload err uint32 The error code returned by the I2C deinitialization function.
 */
class ErrorEventPrimaryDeinitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventPrimaryDeinitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Cleanup, CodeID::PrimaryDeinitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0F030201 ErrorEventSecondaryDeinitFailed
 * @desc The secondary I2C bus deinitialization failed.
 * @fix Ensure that the I2C bus is properly connected and that the pins are correctly configured.
 * @payload err uint32 The error code returned by the I2C deinitialization function.
 */
class ErrorEventSecondaryDeinitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventSecondaryDeinitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Cleanup, CodeID::SecondaryDeinitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0F020101 ErrorEventAddDeviceFailed
 * @desc Adding an I2C device to the bus failed.
 * @fix Ensure that the I2C device is properly connected and that the address is correct.
 * @payload err uint32 The error code returned by the I2C add device function.
 */
class ErrorEventAddDeviceFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventAddDeviceFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Runtime, CodeID::AddDeviceFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0F020201 ErrorEventRemoveDeviceFailed
 * @desc Failed to remove I2C device from the bus.
 * @fix Ensure that the I2C device is properly connected and that the address is correct.
 * @payload err uint32 The error code returned by the I2C remove device function.
 */
class ErrorEventRemoveDeviceFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventRemoveDeviceFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Runtime, CodeID::RemoveDeviceFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0F020301 ErrorEventBufferAllocFailed
 * @desc Failed to allocate memory for I2C write buffer.
 * @fix Ensure that there is sufficient memory available.
 * @payload err uint32 The error code returned by the memory allocation function.
 */
class ErrorEventBufferAllocFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventBufferAllocFailed() :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Runtime, CodeID::BufferAllocFailed, Error::ErrorLevel::Trace)
    {
    }
};

/** <API_REF>
 * @error 0x0F020401 ErrorEventWriteRegistersFailed
 * @desc Failed to write registers to I2C device.
 * @fix Ensure that the I2C device is properly connected and that the address is correct.
 * @payload err uint32 The error code returned by the I2C write function.
 */
class ErrorEventWriteRegistersFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventWriteRegistersFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Runtime, CodeID::WriteRegistersFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0F020501 ErrorEventReadRegistersFailed
 * @desc Failed to read registers from I2C device.
 * @fix Ensure that the I2C device is properly connected and that the address is correct.
 * @payload err uint32 The error code returned by the I2C read function.
 */
class ErrorEventReadRegistersFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventReadRegistersFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::I2C, SubmoduleID::Runtime, CodeID::ReadRegistersFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};