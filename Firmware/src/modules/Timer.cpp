#include "modules/Timer.hpp"
#include "config.hpp"
#include "modules/IMU.hpp"
#include "modules/Motor.hpp"
#include "modules/AnalogScanner.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gptimer.h>

namespace Timer
{
    static TaskHandle_t h_task_adc_800hz = NULL;
    static TaskHandle_t h_task_imu_50hz = NULL;
    static TaskHandle_t h_task_algo_50hz = NULL;
    static TaskHandle_t h_task_motor_50hz = NULL;

    static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
    {
        BaseType_t high_task_awoken = pdFALSE;
        vTaskNotifyGiveFromISR(h_task_adc_800hz, &high_task_awoken);
        return (high_task_awoken == pdTRUE); // return true if a higher priority task was awoken
    }

    void task_adc_800hz(void *pvParameter)
    {
        uint16_t cycle_counter = 0;
        
        while (1) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            AnalogScanner::_update_task(nullptr);
            
            cycle_counter++;
            if (cycle_counter >= TIMER_DIVIDER_SECONDARY) {
                cycle_counter = 0;
                
                // Launch tasks for 50Hz modules
                xTaskNotifyGive(h_task_imu_50hz);
            }
        }
    }

    void task_imu_50hz(void *pvParameter)
    {
        while (1) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            IMU::_update_task(nullptr);
            xTaskNotifyGive(h_task_algo_50hz);
        }
    }

    void task_inference_50hz(void *pvParameter)
    {
        while (1) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            
            // TODO : Inference
            
            xTaskNotifyGive(h_task_motor_50hz);
        }
    }

    void task_motor_50hz(void *pvParameter)
    {
        while (1) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            
            Motor::_update_task(nullptr);
        }
    }

    Error Init()
    {
        // Create the tasks
        xTaskCreate(task_adc_800hz,  "TaskADC",  4096, NULL, 10, &h_task_adc_800hz);
        xTaskCreate(task_imu_50hz,   "TaskIMU",  4096, NULL, 9,  &h_task_imu_50hz); 
        xTaskCreate(task_inference_50hz, "TaskInfer", 8192, NULL, 8,  &h_task_algo_50hz);
        xTaskCreate(task_motor_50hz, "TaskMotor",  4096, NULL, 8,  &h_task_motor_50hz);

        // Configure the hardware timer
        gptimer_handle_t gptimer = NULL;
        gptimer_config_t timer_config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1000000, // 1 MHz, 1 tick = 1 us
        };
        ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

        gptimer_event_callbacks_t cbs = {
            .on_alarm = timer_on_alarm_cb, // link the alarm callback
        };
        ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

        gptimer_alarm_config_t alarm_config = {
            .alarm_count = 1250, // 1250 us = 1.25 ms = 800 Hz
            .reload_count = 0,
            .flags = {
                .auto_reload_on_alarm = true,
            },
        };
        ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

        // Start the timer
        ESP_ERROR_CHECK(gptimer_enable(gptimer));
        ESP_ERROR_CHECK(gptimer_start(gptimer));

        return Error::Ok;
    }
}