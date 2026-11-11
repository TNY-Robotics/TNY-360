#include "locomotion/ControlLoop.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Robot.hpp"
#include "common/Log.hpp"
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

    if (Status err = create_internal_task(); err != Status::Ok)
    {
        return err;
    }
    running = true; // allow the control loop task to run

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

    initialized = true;
    return Status::Ok;
}

Status ControlLoop::deinit()
{
    if (!initialized) return Status::Ok;

    running = false; // ask the control loop to end

    // wait 2 control cycle to be sure the control loop is stopped
    vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_DT_MS*2));

    if (timer_task_handle != nullptr)
    {
        vTaskDelete(timer_task_handle);
        timer_task_handle = nullptr;
    }

    if (esp_err_t err = gptimer_disable(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error disabling Control Loop gptimer ; 0x%0X", err);
        return Status::Failure;
    }

    initialized = false;
    return Status::Ok;
}

Status ControlLoop::start()
{
    if (Status err = create_internal_task(); err != Status::Ok)
    {
        return err;
    }
    running = true; // allow the control loop task to run

    if (esp_err_t err = gptimer_start(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error starting Control Loop timer : 0x%0X", err);
        return Status::Failure;
    }

    return Status::Ok;
}


Status ControlLoop::stop()
{
    running = false; // ask the control loop to end
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

    if (motion_provider != nullptr)
    {
        if (Status err = motion_provider->destroy(); err != Status::Ok)
        {
            return err;
        }
        delete motion_provider;
        motion_provider = nullptr;
    }

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
        running = true;
        
        // Main loop
        while (running)
        {
            // wait for timer interrupt notification
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            control_loop->control_task();
        }

        // clean up and delete task
        running = false;
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