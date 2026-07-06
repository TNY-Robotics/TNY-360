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
    ResetFailed = 0x03,
    ConfigFailed = 0x04,
    WakeUpFailed = 0x05,

    // Runtime
    SendDataFailed = 0x01,

    // Cleanup
    DeleteFailed = 0x01,
};

/** <API_REF>
 * @error 0x07010101 I2CInitFailed
 * @desc Indicates that the I2C initialization failed during initialization.
 * @fix Check that the I2C bus is properly configured and available for use by the Motor driver.
 */
class ErrorEventI2CInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventI2CInitFailed() :
        ErrorEventBuilder(Error::ModuleID::MotorDriver, SubmoduleID::Initialization, CodeID::I2CInitFailed, Error::ErrorSeverity::Trace)
    {
    }
};

/** <API_REF>
 * @error 0x07010201 CreateFailed
 * @desc Indicates that the creation of the PCA9685 handle failed during initialization.
 * @fix Check the I2C connection and ensure that the PCA9685 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventCreateFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCreateFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::MotorDriver, SubmoduleID::Initialization, CodeID::CreateFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x07010301 ResetFailed
 * @desc Indicates that the reset operation of the PCA9685 failed during initialization.
 * @fix Check the I2C connection and ensure that the PCA9685 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventResetFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventResetFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::MotorDriver, SubmoduleID::Initialization, CodeID::ResetFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x07010401 ConfigFailed
 * @desc Indicates that the configuration of the PCA9685 failed during initialization.
 * @fix Check the I2C connection and ensure that the PCA9685 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventConfigFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventConfigFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::MotorDriver, SubmoduleID::Initialization, CodeID::ConfigFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x07010501 WakeUpFailed
 * @desc Indicates that the wake-up operation of the PCA9685 failed during initialization.
 * @fix Check the I2C connection and ensure that the PCA9685 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventWakeUpFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventWakeUpFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::MotorDriver, SubmoduleID::Initialization, CodeID::WakeUpFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x07020101 SendDataFailed
 * @desc Indicates that sending PWM data to the PCA9685 failed during runtime.
 * @fix Check the I2C connection and ensure that the PCA9685 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventSendDataFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventSendDataFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::MotorDriver, SubmoduleID::Runtime, CodeID::SendDataFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x07030101 DeleteFailed
 * @desc Indicates that the deletion of the PCA9685 handle failed during cleanup.
 * @fix Check the I2C connection and ensure that the PCA9685 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventDeleteFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventDeleteFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::MotorDriver, SubmoduleID::Cleanup, CodeID::DeleteFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};