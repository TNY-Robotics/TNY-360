#pragma once
#include "common/Error.hpp"

enum class SubmoduleID : uint8_t
{
    Initialization = 0x01,
    Startup = 0x02,
    Stop = 0x03,
    Runtime = 0x04,
    Cleanup = 0x05,
};

enum class CodeID : uint8_t
{
    // Initialization
    I2CInitFailed = 0x01,
    CameraInitFailed = 0x02,
    GetSensorFailed = 0x03,

    // Startup
    ServerStartFailed = 0x01,
    ServerRegisterFailed = 0x02,

    // Stop
    ServerStopFailed = 0x01,

    // Runtime
    CameraCaptureFailed = 0x01,

    // Cleanup
    CameraDeinitFailed = 0x01,
};

/** <API_REF>
 * @error 0x0D010101 I2CInitFailed
 * @desc The I2C initialization for the camera driver failed.
 * @fix Ensure that the I2C bus is properly configured and available for use by the camera driver.
 */
class ErrorEventI2CInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventI2CInitFailed() :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Initialization, CodeID::I2CInitFailed, Error::ErrorLevel::Trace)
    {
    }
};

/** <API_REF>
 * @error 0x0D010201 CameraInitFailed
 * @desc The camera initialization failed.
 * @fix Ensure that the camera is properly connected, and that the I2C bus is functioning correctly.
 * @payload err esp_err_t The error code returned by the camera initialization function.
 */
class ErrorEventCameraInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCameraInitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Initialization, CodeID::CameraInitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0D010301 GetSensorFailed
 * @desc The camera sensor could not be retrieved after initialization.
 * @fix Ensure that the camera is properly connected, isn't damaged, and that the I2C bus is functioning correctly.
 */
class ErrorEventGetSensorFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventGetSensorFailed() :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Initialization, CodeID::GetSensorFailed, Error::ErrorLevel::Trace)
    {
    }
};

/** <API_REF>
 * @error 0x0D020101 ServerStartFailed
 * @desc The camera web server failed to start.
 * @fix Ensure that the server is properly configured and that there are no port conflicts or other issues preventing it from starting.
 * @payload err uint32 The error code returned by the server start function.
 */
class ErrorEventServerStartFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventServerStartFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Startup, CodeID::ServerStartFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0D020201 ServerRegisterFailed
 * @desc The camera web server failed to register a URI handler.
 * @fix Ensure that the server is properly configured and that there are no memory or resource issues / limits.
 * @payload err uint32 The error code returned by the URI handler registration function.
 */
class ErrorEventServerRegisterFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventServerRegisterFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Startup, CodeID::ServerRegisterFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0D030101 ServerStopFailed
 * @desc The camera web server failed to stop.
 * @fix Ensure that the server is properly configured and that there are no resource issues preventing it from stopping.
 * @payload err uint32 The error code returned by the server stop function.
 */
class ErrorEventServerStopFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventServerStopFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Stop, CodeID::ServerStopFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0D040101 CameraCaptureFailed
 * @desc The camera capture failed.
 * @fix Ensure that the camera is properly connected, isn't damaged, and that the connector and traces on the pcb are clear.
 */
class ErrorEventCameraCaptureFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCameraCaptureFailed() :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Runtime, CodeID::CameraCaptureFailed, Error::ErrorLevel::Trace)
    {
    }
};

/** <API_REF>
 * @error 0x0D050101 CameraDeinitFailed
 * @desc The camera deinitialization failed.
 * @fix Ensure that the camera is properly connected, and that the I2C bus is functioning correctly.
 * @payload err uint32 The error code returned by the camera deinitialization function.
 */
class ErrorEventCameraDeinitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCameraDeinitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Camera, SubmoduleID::Cleanup, CodeID::CameraDeinitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};