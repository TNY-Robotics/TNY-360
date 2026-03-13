#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <functional> // Oubli classique pour std::function :)
#include "common/utils.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

namespace RPC
{
    constexpr const char* TAG = "RPC";

    struct RpcJob
    {
        virtual void execute_on_core_1() = 0;
        virtual void execute_on_core_0() = 0;
        virtual ~RpcJob() = default;
    };

    inline QueueHandle_t rpcRequestQueue = nullptr;  // Core 0 -> Core 1
    inline QueueHandle_t rpcResponseQueue = nullptr; // Core 1 -> Core 0
    inline TaskHandle_t core0_executor_task = nullptr;

    template <typename T>
    struct TypedRpcJob : public RpcJob
    {
        std::function<T()> task_core1;
        std::function<void(T)> callback_core0;
        T result;

        TypedRpcJob(std::function<T()> t, std::function<void(T)> cb) 
            : task_core1(t), callback_core0(cb) {}

        void execute_on_core_1() override
        {
            result = task_core1();
        }

        void execute_on_core_0() override
        {
            callback_core0(result);
        }
    };

    template<typename T>
    Error ExecuteThreadSafe(std::function<T()> task, std::function<void(T)> on_complete)
    {
        if (rpcRequestQueue == nullptr) return Error::InvalidState;

        RpcJob* job = new TypedRpcJob<T>(task, on_complete);
        
        // Send function pointer to Core 1
        if (xQueueSend(rpcRequestQueue, &job, 0) != pdTRUE)
        {
            delete job; // Anti-leak security
            Log::Add(Log::Level::Warning, TAG, "Request queue full, job dropped.");
            return Error::OutOfMemory;
        }

        return Error::None;
    }

    inline void core0_task_func(void* params)
    {
        RpcJob* job;
        while (true)
        {
            // portMAX_DELAY = 0% CPU when idling
            if (xQueueReceive(rpcResponseQueue, &job, portMAX_DELAY) == pdTRUE)
            {
                job->execute_on_core_0();
                delete job; // free task (all done)
            }
        }
    }

    inline void Process_Core1()
    {
        if (rpcRequestQueue == nullptr) return;

        RpcJob* job;
        // Timeout of 0 = non blocking. Max 1 request per 5ms cycle (200Hz)
        if (xQueueReceive(rpcRequestQueue, &job, 0) == pdTRUE)
        {
            job->execute_on_core_1();
            xQueueSend(rpcResponseQueue, &job, 0); // Send back the job to core 0
        }
    }

    /**
     * @brief Initializes the RPC layer (job queues, executor tasks, etc.)
     */
    inline Error Init()
    {
        // RPC jobs queue creation
        rpcRequestQueue = xQueueCreate(RPC_QUEUE_SIZE, sizeof(RpcJob*));
        rpcResponseQueue = xQueueCreate(RPC_QUEUE_SIZE, sizeof(RpcJob*));

        if (rpcRequestQueue == nullptr || rpcResponseQueue == nullptr) {
            Log::Add(Log::Level::Error, TAG, "Failed to create RPC queues");
            return Error::Unknown; // Remplace par ton code d'erreur
        }

        // Launching core 0 background job task
        if (xTaskCreatePinnedToCore(core0_task_func, "RPC_Core0", 4096, nullptr, tskIDLE_PRIORITY + 5, &core0_executor_task, CORE_BRAIN) != pdPASS)
        {
            Log::Add(Log::Level::Error, TAG, "Error creating Core0 thread safe executor");
            return Error::Unknown;
        }
        
        // NOTE : Not creating any task on core 1
        // RPC::Process_Core1() should be executed in the control loop

        return Error::None;
    }

    /**
     * @brief Deinitializes the RPC layer (stop executor tasks, delete queues, etc.)
     */
    inline Error DeInit()
    {
        if (core0_executor_task != nullptr)
        {
            vTaskDelete(core0_executor_task);
            core0_executor_task = nullptr;
        }
        if (rpcRequestQueue != nullptr) vQueueDelete(rpcRequestQueue);
        if (rpcResponseQueue != nullptr) vQueueDelete(rpcResponseQueue);
        
        return Error::None;
    }
}