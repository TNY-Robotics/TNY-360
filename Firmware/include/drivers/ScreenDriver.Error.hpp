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
    PanelCreateFailed = 0x02,
    PanelResetFailed = 0x03,
    PanelInitFailed = 0x04,
    PanelDisplayOnFailed = 0x05,

    // Runtime
    UploadFailed = 0x01,

    // Cleanup
    PanelDeleteFailed = 0x01,
};

/** <API_REF>
 * @error 0x15010101 I2CInitFailed
 * @desc Indicates that the I2C initialization failed during initialization.
 * @fix Check that the I2C bus is properly configured and available for use by the Screen driver.
 */
class ErrorEventI2CInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventI2CInitFailed() :
        ErrorEventBuilder(Error::ModuleID::Screen, SubmoduleID::Initialization, CodeID::I2CInitFailed, Error::ErrorLevel::Trace)
    {
    }
};

/** <API_REF>
 * @error 0x15010201 PanelCreateFailed
 * @desc Indicates that the creation of the screen panel failed during initialization.
 * @fix Check the I2C connection and ensure that the screen is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventPanelCreateFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventPanelCreateFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Screen, SubmoduleID::Initialization, CodeID::PanelCreateFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x15010301 PanelResetFailed
 * @desc Indicates that the reset of the screen panel failed during initialization.
 * @fix Check the I2C connection and ensure that the screen is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventPanelResetFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventPanelResetFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Screen, SubmoduleID::Initialization, CodeID::PanelResetFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x15010401 PanelInitFailed
 * @desc Indicates that the initialization of the screen panel failed during initialization.
 * @fix Check the I2C connection and ensure that the screen is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventPanelInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventPanelInitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Screen, SubmoduleID::Initialization, CodeID::PanelInitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x15010501 PanelDisplayOnFailed
 * @desc Indicates that turning on the display of the screen panel failed during initialization.
 * @fix Check the I2C connection and ensure that the screen is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventPanelDisplayOnFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventPanelDisplayOnFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Screen, SubmoduleID::Initialization, CodeID::PanelDisplayOnFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x15020101 UploadFailed
 * @desc Indicates that uploading the screen buffer to the screen panel failed during runtime.
 * @fix Check the I2C connection and ensure that the screen is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventUploadFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventUploadFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Screen, SubmoduleID::Runtime, CodeID::UploadFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x15030101 PanelDeleteFailed
 * @desc Indicates that deleting the screen panel failed during cleanup.
 * @fix Check the I2C connection and ensure that the screen is properly connected and powered.
 * @payload err uint32 The error code returned by the failed operation.
 */
class ErrorEventPanelDeleteFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventPanelDeleteFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Screen, SubmoduleID::Cleanup, CodeID::PanelDeleteFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};