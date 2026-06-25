#include "locomotion/IPC.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "common/Log.hpp"

namespace IPC
{
    // NOTE : Using DRAM_ATTR to make sure the control loop won't be blocked by cache issues when accessing the queues
    //        (on high wifi usage or other flash access heavy operations)
    DRAM_ATTR static uint8_t intent_queue_buffer[sizeof(ControlIntent)];
    DRAM_ATTR static StaticQueue_t intent_queue_struct;
    QueueHandle_t intent_queue = nullptr;
    DRAM_ATTR static uint8_t state_queue_buffer[sizeof(RobotState)];
    DRAM_ATTR static StaticQueue_t state_queue_struct;
    QueueHandle_t state_queue = nullptr;

    Status Init()
    {
        LOG_SCOPE(TAG, "IPC::Init");
        
        intent_queue = xQueueCreateStatic(1, sizeof(ControlIntent), intent_queue_buffer, &intent_queue_struct);
        state_queue = xQueueCreateStatic(1, sizeof(RobotState), state_queue_buffer, &state_queue_struct);

        if (intent_queue == nullptr || state_queue == nullptr)
        {
            LOG_ERROR(TAG, "Failed to create queues for IPC");
            // ErrorHandle(ErrorStruct::IPCInitFailed);
            return Status::Failure;
        }

        return Status::Ok;
    }

    Status Deinit()
    {
        return Status::Ok;
    }

    Status setIntent(ControlIntent& intent)
    {
        if (xQueueOverwrite(intent_queue, &intent) != pdPASS)
        {
            LOG_ERROR(TAG, "Couldn't override intent in the intent queue");
            return Status::Failure;
        }
        return Status::Ok;
    }

    bool getIntent(ControlIntent* intent)
    {
        return (xQueueReceive(intent_queue, intent, 0) == pdTRUE);
    }

    Status setState(RobotState& state)
    {
        if (xQueueOverwrite(state_queue, &state) != pdPASS)
        {
            LOG_ERROR(TAG, "Couldn't override state in the state queue");
            return Status::Failure;
        }
        return Status::Ok;
    }

    bool getState(RobotState* state)
    {
        return (xQueueReceive(state_queue, state, 0) == pdTRUE);
    }
};