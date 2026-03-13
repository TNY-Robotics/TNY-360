#include "decision/DecisionLoop.hpp"
#include "esp_log_timestamp.h"
#include "common/config.hpp"
#include "common/Log.hpp"

Error DecisionLoop::init()
{
    return Error::None;
}

Error DecisionLoop::deinit()
{
    if (decision_loop_task != nullptr)
    {
        vTaskDelete(decision_loop_task);
        decision_loop_task = nullptr;
    }

    return Error::None;
}

Error DecisionLoop::start()
{
    if (xTaskCreatePinnedToCore([](void* pvParams){
        DecisionLoop* decision_loop = (DecisionLoop*) pvParams;
        decision_loop->decision_loop();
    }, "DecisionLoop_Core0", 8192, this, tskIDLE_PRIORITY + 10, &decision_loop_task, CORE_BRAIN) != pdPASS)
    {
        Log::Add(Log::Level::Error, TAG, "Error creating DecisionLoop task");
        return Error::Unknown;
    }

    return Error::None;
}

Error DecisionLoop::stop()
{
    loop_running = false;

    // wait 2 decision cycles to be sure it's stopped
    vTaskDelay(pdMS_TO_TICKS(DECISION_LOOP_DT_MS * 2));

    return Error::None;
}

void DecisionLoop::setBodyVelocity(Vec3f vel)
{
    intent.body_vel = vel;
}

void DecisionLoop::decision_loop()
{
    loop_running = true;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(DECISION_LOOP_DT_MS);

    while (loop_running)
    {

        // Update timestamp and send intent to REFLEX core
        intent.timestamp_ms = esp_log_timestamp();
        if (IPC::setIntent(intent) != Error::None)
        {
            Log::Add(Log::Level::Warning, TAG, "Failed to send intent to Reflex core");
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    if (decision_loop_task != nullptr)
    {
        vTaskDelete(nullptr);
        decision_loop_task = nullptr;
    }
}
