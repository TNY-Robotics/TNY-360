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
    ReadDataFailed = 0x01,

    //Cleanup
    DeleteFailed = 0x01,
};

/** <API_REF>
 * @error 0x09010101 I2CInitFailed
 * @desc Indicates that the I2C initialization failed during initialization.
 * @fix Check that the I2C bus is properly configured and available for use by the IMU driver.
 */
class ErrorEventI2CInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventI2CInitFailed() :
        ErrorEventBuilder(Error::ModuleID::IMU, SubmoduleID::Initialization, CodeID::I2CInitFailed, Error::ErrorLevel::Trace)
    {
    }
};

/** <API_REF>
 * @error 0x09010201 CreateFailed
 * @desc Indicates that the creation of the MPU6050 handle failed during initialization.
 * @fix Check the I2C connection and ensure that the MPU6050 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventCreateFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCreateFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::IMU, SubmoduleID::Initialization, CodeID::CreateFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x09010301 ResetFailed
 * @desc Indicates that the reset operation of the MPU6050 failed during initialization.
 * @fix Check the I2C connection and ensure that the MPU6050 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventResetFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventResetFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::IMU, SubmoduleID::Initialization, CodeID::ResetFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x09010401 ConfigFailed
 * @desc Indicates that the configuration of the MPU6050 failed during initialization.
 * @fix Check the I2C connection and ensure that the MPU6050 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventConfigFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventConfigFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::IMU, SubmoduleID::Initialization, CodeID::ConfigFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x09010501 WakeUpFailed
 * @desc Indicates that the wake-up operation of the MPU6050 failed during initialization.
 * @fix Check the I2C connection and ensure that the MPU6050 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventWakeUpFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventWakeUpFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::IMU, SubmoduleID::Initialization, CodeID::WakeUpFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x09020101 ReadDataFailed
 * @desc Indicates that reading data from the MPU6050 failed during runtime.
 * @fix Check the I2C connection and ensure that the MPU6050 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventReadDataFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventReadDataFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::IMU, SubmoduleID::Runtime, CodeID::ReadDataFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x09030101 DeleteFailed
 * @desc Indicates that the deletion of the MPU6050 handle failed during cleanup.
 * @fix Check the I2C connection and ensure that the MPU6050 is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventDeleteFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventDeleteFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::IMU, SubmoduleID::Cleanup, CodeID::DeleteFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};
