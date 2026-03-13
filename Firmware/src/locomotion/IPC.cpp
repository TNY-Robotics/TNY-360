#include "locomotion/IPC.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "common/Log.hpp"

namespace IPC
{
    QueueHandle_t intent_queue = nullptr;

    Error Init()
    {
        intent_queue = xQueueCreate(1, sizeof(ControlIntent));

        if (intent_queue == nullptr)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to create intent queue");
            return Error::SoftwareFailure;
        }

        return Error::None;
    }

    Error Deinit()
    {
        return Error::None;
    }

    Error setIntent(ControlIntent& intent)
    {
        if (xQueueOverwrite(intent_queue, &intent) != pdPASS)
        {
            Log::Add(Log::Level::Error, TAG, "Couldn't override intent in the intent queue");
            return Error::SoftwareFailure;
        }
        return Error::None;
    }

    bool getIntent(ControlIntent* intent)
    {
        return (xQueueReceive(intent_queue, intent, 0) == pdTRUE);
    }
};