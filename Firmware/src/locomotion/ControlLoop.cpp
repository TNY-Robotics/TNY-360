#include "locomotion/ControlLoop.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Robot.hpp"
#include "common/Log.hpp"
#include "common/RPC.hpp"
#include "common/config.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"

TaskHandle_t timer_task_handle;
bool running = false;

static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    vTaskNotifyGiveFromISR(timer_task_handle, &high_task_awoken);
    return (high_task_awoken == pdTRUE); // return true if a higher priority task was awoken
}

ControlLoop::ControlLoop()
    : initialized(false)
{
}

Status ControlLoop::init()
{
    // FIXME : It would really be better if we made sure everything is in DRAM
    //         to avoid cache issues and delays in the control loop when brain core is doing heavy operations

    if (initialized) return Status::Ok;

    // Configure the timer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION,
        .intr_priority = 0, // Let driver choose a low priority
        .flags = {
            .intr_shared = false, // Don't share the interrupt
            .allow_pd = false, // Don't allow power down
        },
    };
    if (esp_err_t err = gptimer_new_timer(&timer_config, &timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_new_timer failed");
        return Status::Failure;
    }

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb, // link the alarm callback
    };
    if (esp_err_t err = gptimer_register_event_callbacks(timer, &cbs, NULL); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_register_event_callbacks failed");
        return Status::Failure;
    }

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = TIMER_ALARM_COUNT,
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true,
        },
    };
    if (esp_err_t err = gptimer_set_alarm_action(timer, &alarm_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_set_alarm_action failed");
        return Status::Failure;
    }

    // Enable the timer
    if (esp_err_t err = gptimer_enable(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_enable failed");
        return Status::Failure;
    }

    running = false;
    initialized = true;
    return Status::Ok;
}

Status ControlLoop::deinit()
{
    if (!initialized) return Status::InvalidState;

    // ask the control loop to stop (in case it was running)
    if (running)
    {
        if (Status err = stop(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to stop Control Loop");
            return err;
        }
    }

    // disable the timer and free resources
    if (esp_err_t err = gptimer_disable(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error disabling Control Loop gptimer ; 0x%0X", err);
        return Status::Failure;
    }
    if (esp_err_t err = gptimer_del_timer(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error deleting Control Loop gptimer ; 0x%0X", err);
        return Status::Failure;
    }

    // if there's a motion provider, remove it
    if (motion_provider != nullptr)
    {
        if (Status err = motion_provider->destroy(); err != Status::Ok)
        {
            return err;
        }
        delete motion_provider;
        motion_provider = nullptr;
    }

    initialized = false;
    return Status::Ok;
}

Status ControlLoop::start()
{
    if (!initialized) return Status::InvalidState;
    if (running) return Status::Ok;

    running = true; // allow the control loop task to run

    if (Status err = create_internal_task(); err != Status::Ok)
    {
        return err;
    }

    // STart the timer to trigger the control loop task
    if (esp_err_t err = gptimer_start(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error starting Control Loop timer : 0x%0X", err);
        return Status::Failure;
    }

    return Status::Ok;
}


Status ControlLoop::stop()
{
    if (!initialized) return Status::InvalidState;
    if (!running) return Status::Ok;
    running = false; // ask the control loop to end

    // wait 2 control cycle to be sure the control loop is stopped
    vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_DT_MS*2));
    
    // No need to delete the task, it will delete itself

    // stop the timer
    if (esp_err_t err = gptimer_stop(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error stopping Control Loop timer : 0x%0X", err);
        return Status::Failure;
    }

    return Status::Ok;
}

bool ControlLoop::isRunning() const
{
    return running;
}

Status ControlLoop::setMotionProvider(MotionProvider* provider)
{
    if (!initialized) return Status::InvalidState;
    
    // Whether we are on the REFLEX core, or we are on the BRAIN core
    // We want to set the new motion provider at the end of the current control loop tick
    // (and the RPC calls are executed at the end of the control loop tick)
    // NOTE : Yes, this is a "dirty" trick, but is it really worth it to do it in an other way ?
    RPC::ExecuteThreadSafe<Status>([this, provider]() {
        return set_motion_provider(provider);
    }, [](Status s) {});
    return Status::Ok;
}

Status ControlLoop::set_motion_provider(MotionProvider* provider)
{
    if (!initialized) return Status::InvalidState;

    // if there's a motion provider, remove it
    if (motion_provider != nullptr)
    {
        if (Status err = motion_provider->destroy(); err != Status::Ok)
        {
            return err;
        }
        delete motion_provider;
        motion_provider = nullptr;
    }

    // then, add the new motion provider
    motion_provider = provider;
    if (motion_provider != nullptr)
    {
        if (Status err = motion_provider->create(); err != Status::Ok)
        {
            return err;
        }
    }


    return Status::Ok;
}

Status ControlLoop::create_internal_task()
{
    // Create the control loop task ON REFLEX CORE
    BaseType_t err = xTaskCreatePinnedToCore([](void* PvParams){
        ControlLoop* control_loop = static_cast<ControlLoop*>(PvParams);
        running = true; // just in case, set the running flag to true (it should already be true)
        
        // Main loop
        while (running)
        {
            // wait for timer interrupt notification
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            // execute the control loop task
            control_loop->control_task();
        }

        // clean up and delete task
        running = false; // just in case
        if (timer_task_handle != nullptr)
        {
            vTaskDelete(nullptr);
            timer_task_handle = nullptr;
        }
    },  "timer_task", 8192, this, configMAX_PRIORITIES - 1, &timer_task_handle, CORE_REFLEX);

    if (err != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create timer task");
        // ErrorHandle(ErrorStruct::ControlLoopInitFailed);
        return Status::Failure;
    }

    return Status::Ok;
}