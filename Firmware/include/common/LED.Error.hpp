#pragma once
#include "common/LED.hpp"
#include "common/Error.hpp"
#include "esp_err.h"

namespace LED
{
    enum class SubmoduleID : uint8_t
    {
        Initialization = 0x01,
    };

    enum class CodeID : uint8_t
    {
        // Initialization
        TXChannel = 0x01,
        Encoder = 0x02,
        Enable = 0x03,
        Task = 0x04,
    };

    /** <API_REF>
     * @error 0x14010101 TXChannel
     * @desc Failed to initialize the LED TX channel (RMT).
     * @fix Check if the RMT peripheral is already in use or if the memory is exhausted.
     * @payload err uint32 The underlying ESP-IDF error code.
     */
    class ErrorEventTXChannelInit : public Error::ErrorEventBuilder
    {
    public:
        ErrorEventTXChannelInit(esp_err_t err) :
            ErrorEventBuilder(Error::ModuleID::LED, SubmoduleID::Initialization, CodeID::TXChannel, Error::ErrorLevel::Trace)
        {
            appendPayload(err);
        }
    };

    /** <API_REF>
     * @error 0x14010201 Encoder
     * @desc Failed to initialize the WS2812 bytes encoder.
     * @fix Check the WS2812 timing configuration parameters.
     * @payload err uint32 The underlying ESP-IDF error code.
     */
    class ErrorEventEncoderInit : public Error::ErrorEventBuilder
    {
    public:
        ErrorEventEncoderInit(esp_err_t err) :
            ErrorEventBuilder(Error::ModuleID::LED, SubmoduleID::Initialization, CodeID::Encoder, Error::ErrorLevel::Trace)
        {
            appendPayload(err);
        }
    };

    /** <API_REF>
     * @error 0x14010301 Enable
     * @desc Failed to enable the LED TX channel (RMT).
     * @fix Check if the RMT peripheral is already in use or if the memory is exhausted.
     * @payload err uint32 The underlying ESP-IDF error code.
     */
    class ErrorEventEnable : public Error::ErrorEventBuilder
    {
    public:
        ErrorEventEnable(esp_err_t err) :
            ErrorEventBuilder(Error::ModuleID::LED, SubmoduleID::Initialization, CodeID::Enable, Error::ErrorLevel::Trace)
        {
            appendPayload(err);
        }
    };

    /** <API_REF>
     * @error 0x14010401 Task
     * @desc Failed to create the LED update task.
     * @fix Check if the FreeRTOS task creation failed due to insufficient memory or priority issues.
     * @payload err uint32 The underlying FreeRTOS error code.
     */
    class ErrorEventTaskInit : public Error::ErrorEventBuilder
    {
    public:
        ErrorEventTaskInit(BaseType_t err) :
            ErrorEventBuilder(Error::ModuleID::LED, SubmoduleID::Initialization, CodeID::Task, Error::ErrorLevel::Trace)
        {
            appendPayload(err);
        }
    };
}