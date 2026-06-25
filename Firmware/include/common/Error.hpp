#pragma once
#include <cstdint>
#include <memory.h>

namespace Error
{
    enum class ModuleID : uint8_t
    {
        System = 0x01,
        Protocol = 0x02,
        Gait = 0x03,
        Body = 0x04,
        Leg = 0x05,
        Joint = 0x06,
        MotorDriver = 0x07,
        MotorController = 0x08,
        IMU = 0x09,
        Laser = 0x0A,
        Power = 0x0B,
        Face = 0x0C,
        Camera = 0x0D,
        Speaker = 0x0E,
        Microphone = 0x0F,
        I2C = 0x10,
        ADC = 0x11,
        WiFi = 0x12,
        Bluetooth = 0x13,
        LED = 0x14,
        Screen = 0x15,
        Error = 0x16,
    };

    /** <API_REF>
     * @type ErrorLevel
     * @desc Represents the severity level of an error event.
     * @value Trace Low level problems (debug infos that would be useful in case of problems)
     * @value Warning When non-critical issues occur that may cause further problems
     * @value Error When important issues occur that will degrade normal operation
     * @value Critical When critical issues occur, and the system should stop to prevent further damage
     */
    enum class ErrorLevel : uint8_t
    {
        // Low level problems (debug infos that would be useful in case of problems)
        Trace = 0x01,
        // When non-critical issues occur that may cause further problems
        Warning = 0x02,
        // When important issues occur that will degrade normal operation
        Error = 0x03,
        // When critical issues occur, and the system should stop to prevent further damage
        Critical = 0x04
    };

    /** <API_REF>
     * @type ErrorEvent
     * @desc Represents an error event that occurred in the system.
     * @field timestampMs uint32 The timestamp of the error event in milliseconds since system start.
     * @field error struct The error code information
     * @field error.fullCode uint32 The full error code, which can be used to uniquely identify the error.
     * @field error.parts struct The individual parts of the error code.
     * @field error.parts.module uint8 The module ID where the error occurred (see ModuleID).
     * @field error.parts.subsystem uint8 The subsystem ID within the module where the error occurred.
     * @field error.parts.code uint8 The specific error code within the subsystem.
     * @field error.parts.category ErrorLevel The severity level of the error (see ErrorLevel).
     * @field eventId uint16 The unique event ID assigned to this error event.
     * @field payloadSize uint8 The size of the payload data in bytes.
     * @field payload uint8[32] The payload data associated with the error event (up to 32 bytes).
     */
    struct ErrorEvent
    {
        uint32_t timestampMs = 0;

        union
        {
            uint32_t fullCode = 0;
            struct
            {
                uint8_t module = 0;
                uint8_t subsystem = 0;
                uint8_t code = 0;
                ErrorLevel category = ErrorLevel::Warning;
            } parts;
        } error;

        uint16_t eventId = 0;

        uint8_t payloadSize = 0;
        uint8_t payload[32] = {0};
    };

    class ErrorEventBuilder
    {
    public:
        ErrorEventBuilder();
        ErrorEventBuilder(uint8_t module, uint8_t subsystem, uint8_t code, ErrorLevel category);

        template <typename TModule, typename TSubsystem, typename TCode>
        ErrorEventBuilder(TModule module, TSubsystem subsystem, TCode code, ErrorLevel category)
        : ErrorEventBuilder(static_cast<uint8_t>(module), static_cast<uint8_t>(subsystem), static_cast<uint8_t>(code), category)
        {}

        ErrorEventBuilder& setModule(uint8_t module);
        ErrorEventBuilder& setSubsystem(uint8_t subsystem);
        ErrorEventBuilder& setCode(uint8_t code);
        ErrorEventBuilder& setCategory(ErrorLevel category);

        template <typename T>
        ErrorEventBuilder& setModule(T module) { return setModule(static_cast<uint8_t>(module)); }
        template <typename T>
        ErrorEventBuilder& setSubsystem(T subsystem) { return setSubsystem(static_cast<uint8_t>(subsystem)); }
        template <typename T>
        ErrorEventBuilder& setCode(T code) { return setCode(static_cast<uint8_t>(code)); }
        template <typename T>
        ErrorEventBuilder& setCategory(T category) { return setCategory(static_cast<uint8_t>(category)); }

        template <typename T>
        ErrorEventBuilder& appendPayload(const T& data)
        {
            size_t size = sizeof(T);
            if (errorEvent.payloadSize + size <= sizeof(errorEvent.payload)) {
                memcpy(&errorEvent.payload[errorEvent.payloadSize], &data, size);
                errorEvent.payloadSize += size;
            }
            return *this;
        }

        const ErrorEvent& getErrorEvent() const;
    protected:
        ErrorEvent errorEvent;
    };

    /**
     * @brief Register an error event in the system.
     * @param event The ErrorEvent to register.
     * @return The event ID assigned to the registered error event.
     * @note You can use ErrorEventBuilder to construct a custom error event with specific module, subsystem, and error IDs.
     * @note The system maintains a buffer of recent error events, which can be retrieved using GetErrorEvent().
     * @note If the buffer is full, the oldest error event will be overwritten.
     */
    uint16_t RegisterErrorEvent(const ErrorEvent& event);

    /**
     * @brief Register an error event in the system.
     * @param event The ErrorEventBuilder to register (will be used to get the ErrorEvent).
     * @return The event ID assigned to the registered error event.
     * @note You can use ErrorEventBuilder to construct a custom error event with specific module, subsystem, and error IDs.
     * @note The system maintains a buffer of recent error events, which can be retrieved using GetErrorEvent().
     * @note If the buffer is full, the oldest error event will be overwritten.
     */
    uint16_t RegisterErrorEvent(const ErrorEventBuilder& event);

    /**
     * @brief Get the total number of error events currently stored in the buffer (up to ERROR_MAX_EVENTS).
     * @return The number of error events currently stored.
     */
    uint16_t GetErrorCount();
    
    /**
     * @brief Get an ErrorEvent by its index, where 0 is the most recent event, 1 is the previous event, and so on.
     * @param index The index of the error event to retrieve (0 for most recent)
     * @return A reference to the ErrorEvent struct containing the error event information.
     * @note If the index is out of bounds (greater than or equal to the number of stored error events), an empty event with fullCode == 0 will be returned.
     */
    const ErrorEvent& GetErrorEventByIndex(uint16_t index);

    /**
     * @brief Get an ErrorEvent by its event ID.
     * @param eventId The event ID of the error event to retrieve.
     * @return A reference to the ErrorEvent struct containing the error event information.
     * @note If the event ID is not found, an empty event with fullCode == 0 will be returned.
     */
    const ErrorEvent& GetErrorEventById(uint16_t eventId);
};
