#pragma once
#include "common/Error.hpp"
#include "esp_err.h"

enum class SubmoduleID : uint8_t
{
    SoundMixer = 0x01,
    Speaker = 0x02,
};

enum class CodeID : uint8_t
{
    // SoundMixer
    CreateTaskFailed = 0x01,

    // Speaker
    CreateI2SFailed = 0x01,
    InitPDMFailed = 0x02,
    EnableI2SFailed = 0x03,

    DisableI2SFailed = 0x04,
    DeleteI2SFailed = 0x05,
};

/** <API_REF>
 * @error 0x0E010101 CreateTaskFailed
 * @module Speaker
 * @submodule SoundMixer
 * @severity Trace
 * @desc The SoundMixer background task could not be created. 
 * @fix Ensure that there are sufficient resources available to create the task (stack size, heap memory, etc.)
 * @payload err uint32 The FreeRTOS error code returned by the task creation function.
 */
class ErrorEventCreateTaskFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCreateTaskFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Speaker, SubmoduleID::SoundMixer, CodeID::CreateTaskFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0E020101 CreateI2SFailed
 * @module Speaker
 * @submodule Speaker
 * @severity Trace
 * @desc The Speaker I2S channel could not be created.
 * @fix Ensure that there are sufficient resources available to create the channel (DMA buffers, etc.)
 * @payload err uint32 The ESP-IDF error code returned by the channel creation function.
 */
class ErrorEventCreateI2SFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCreateI2SFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Speaker, SubmoduleID::Speaker, CodeID::CreateI2SFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0E020102 InitPDMFailed
 * @module Speaker
 * @submodule Speaker
 * @severity Trace
 * @desc The Speaker PDM TX mode could not be initialized.
 * @fix Ensure that the I2S channel is properly configured and that there are sufficient resources available to initialize PDM TX mode.
 * @payload err uint32 The ESP-IDF error code returned by the PDM TX initialization function.
 */
class ErrorEventInitPDMFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventInitPDMFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Speaker, SubmoduleID::Speaker, CodeID::InitPDMFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0E020103 EnableI2SFailed
 * @module Speaker
 * @submodule Speaker
 * @severity Trace
 * @desc The Speaker I2S channel could not be enabled.
 * @fix Ensure that the I2S channel is properly configured and that there are sufficient resources available to enable the channel.
 * @payload err uint32 The ESP-IDF error code returned by the channel enable function.
 */
class ErrorEventEnableI2SFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventEnableI2SFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Speaker, SubmoduleID::Speaker, CodeID::EnableI2SFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0E020104 DisableI2SFailed
 * @module Speaker
 * @submodule Speaker
 * @severity Trace
 * @desc The Speaker I2S channel could not be disabled.
 * @fix Ensure that the I2S channel is properly configured and that there are sufficient resources available to disable the channel.
 * @payload err uint32 The ESP-IDF error code returned by the channel disable function.
 */
class ErrorEventDisableI2SFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventDisableI2SFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Speaker, SubmoduleID::Speaker, CodeID::DisableI2SFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x0E020105 DeleteI2SFailed
 * @module Speaker
 * @submodule Speaker
 * @severity Trace
 * @desc The Speaker I2S channel could not be deleted.
 * @fix Ensure that the I2S channel is properly configured and that there are sufficient resources available to delete the channel.
 * @payload err uint32 The ESP-IDF error code returned by the channel delete function.
 */
class ErrorEventDeleteI2SFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventDeleteI2SFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::Speaker, SubmoduleID::Speaker, CodeID::DeleteI2SFailed, Error::ErrorSeverity::Trace)
    {
        appendPayload(err);
    }
};