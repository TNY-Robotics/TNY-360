#include "locomotion/Timer.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gptimer.h"
#include "common/config.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include "Robot.hpp"

TaskHandle_t timer_task_handle;
int call_counter = 0;
bool running = false;

static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    vTaskNotifyGiveFromISR(timer_task_handle, &high_task_awoken);
    return (high_task_awoken == pdTRUE); // return true if a higher priority task was awoken
}

Timer::Timer()
    : initialized(false)
{
}

Error Timer::init()
{
    if (initialized) return Error::None;

    // Create the tasks
    BaseType_t err = xTaskCreate([](void* PvParams){
        Timer* timer = static_cast<Timer*>(PvParams);
        running = true;
        
        // Main loop
        while (running)
        {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait for timer interrupt notification

            timer->analog_read_task();
            if (call_counter++ == TIMER_DIVIDER_SECONDARY)
            {
                call_counter = 0;
                timer->control_task();
            }
        }

        // clean up and delete task
        running = false;
        vTaskDelete(nullptr);
    },  "timer_task",  4096, this, tskIDLE_PRIORITY + 10, &timer_task_handle);

    if (err != pdPASS)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to create timer task");
        return Error::SoftwareFailure;
    }

    // Configure the hardware timer
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz, 1 tick = 1 us
    };
    if (esp_err_t err = gptimer_new_timer(&timer_config, &gptimer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_new_timer failed");
        return Error::SoftwareFailure;
    }

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb, // link the alarm callback
    };
    if (esp_err_t err = gptimer_register_event_callbacks(gptimer, &cbs, NULL); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_register_event_callbacks failed");
        return Error::SoftwareFailure;
    }

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1250, // 800 Hz -> 1.25 ms -> 1250 us
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true,
        },
    };
    if (esp_err_t err = gptimer_set_alarm_action(gptimer, &alarm_config); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_set_alarm_action failed");
        return Error::SoftwareFailure;
    }

    // Start the timer
    if (esp_err_t err = gptimer_enable(gptimer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_enable failed");
        return Error::SoftwareFailure;
    }
    if (esp_err_t err = gptimer_start(gptimer); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "gptimer_start failed");
        return Error::SoftwareFailure;
    }

    initialized = true;
    return Error::None;
}

Error Timer::deinit()
{
    if (!initialized) return Error::None;

    running = false; // stop the task loop

    initialized = false;
    return Error::None;
}

Error Timer::analog_read_task()
{
    AnalogDriver::__ISRStepRead();
    return Error::None;
}

Error Timer::control_task()
{
    // Read the IMU data
    // TODO : IMU::__ISRReadData();

    // Update the body
    if (Error err = Robot::GetInstance().getBody().update(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Body update failed in control task with error: %d", static_cast<uint8_t>(err));
        return err;
    }

    // Send the motor values
    MotorDriver::__ISRSendValues();
    return Error::None;
}