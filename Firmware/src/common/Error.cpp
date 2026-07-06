#include "common/utils.hpp"
#include "common/config.hpp"
#include "common/LED.hpp"
#include "ui/Menus.hpp"
#include <esp_timer.h>
#include <mutex>

namespace Error
{
    ErrorEventBuilder::ErrorEventBuilder()
    {
        errorEvent.module = 0;
        errorEvent.subsystem = 0;
        errorEvent.code = 0;
        errorEvent.severity = ErrorSeverity::Trace;
        errorEvent.payloadSize = 0;
    }

    ErrorEventBuilder::ErrorEventBuilder(uint8_t module, uint8_t subsystem, uint8_t code, ErrorSeverity severity)
    {
        errorEvent.code = code;
        errorEvent.module = module;
        errorEvent.subsystem = subsystem;
        errorEvent.severity = severity;
        errorEvent.payloadSize = 0;
    }

    ErrorEventBuilder& ErrorEventBuilder::setModule(uint8_t module)
    {
        errorEvent.module = module;
        return *this;
    }

    ErrorEventBuilder& ErrorEventBuilder::setSubsystem(uint8_t subsystem)
    {
        errorEvent.subsystem = subsystem;
        return *this;
    }

    ErrorEventBuilder& ErrorEventBuilder::setCode(uint8_t code)
    {
        errorEvent.code = code;
        return *this;
    }

    ErrorEventBuilder& ErrorEventBuilder::setSeverity(ErrorSeverity severity)
    {
        errorEvent.severity = severity;
        return *this;
    }

    const ErrorEvent& ErrorEventBuilder::getErrorEvent() const
    {
        return errorEvent;
    }

    
    static ErrorEvent ringBuffer[ERROR_MAX_EVENTS];
    static uint16_t head = 0;
    static uint16_t count = 0;
    static uint16_t nextEventId = 1;
    static std::mutex errorMutex;
    
    static const ErrorEvent emptyEvent = {
        .timestampMs = 0,
        .eventId = 0,
        .module = 0,
        .subsystem = 0,
        .code = 0,
        .severity = ErrorSeverity::Trace,
        .payloadSize = 0,
        .payload = {0}
    };

    uint16_t RegisterErrorEvent(const ErrorEvent& event)
    {   
        // dual-core architecture, better be safe here
        std::lock_guard<std::mutex> lock(errorMutex);

        ErrorEvent finalEvent = event;

        finalEvent.eventId = nextEventId++;
        finalEvent.timestampMs = esp_timer_get_time() / 1000;

        ringBuffer[head] = finalEvent;
        head = (head + 1) % ERROR_MAX_EVENTS;
        if (count < ERROR_MAX_EVENTS) count++;

        return finalEvent.eventId;
    }

    uint16_t RegisterErrorEvent(const ErrorEventBuilder& builder)
    {
        return RegisterErrorEvent(builder.getErrorEvent());
    }

    uint16_t GetErrorCount()
    {
        std::lock_guard<std::mutex> lock(errorMutex);
        return count;
    }

    const ErrorEvent& GetErrorEventByIndex(uint16_t index)
    {
        std::lock_guard<std::mutex> lock(errorMutex);

        if (index >= count) return emptyEvent;

        int16_t actualIndex = head - 1 - index;
        
        if (actualIndex < 0)
        {
            actualIndex += ERROR_MAX_EVENTS;
        }

        return ringBuffer[actualIndex];
    }

    const ErrorEvent& GetErrorEventById(uint16_t eventId)
    {
        std::lock_guard<std::mutex> lock(errorMutex);

        for (uint16_t i = 0; i < count; i++)
        {
            if (ringBuffer[i].eventId == eventId)
            {
                return ringBuffer[i];
            }
        }
        return emptyEvent;
    }

    void ClearErrorEvents()
    {
        std::lock_guard<std::mutex> lock(errorMutex);
        head = 0;
        count = 0;
        nextEventId = 1;
    }
}