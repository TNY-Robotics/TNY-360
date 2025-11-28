#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "modules/Buttons.hpp"
#include "modules/Log.hpp"

namespace Buttons
{
    bool btn_states[2] = {false, false};
    bool long_pressed[2] = {0, 0};
    TickType_t last_press[2] = {0, 0};

    CallbackSet callbacks = { {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr} };

    void update_task(void* pvParameters)
    {
        bool btn_states_now[2] = {
            (bool) gpio_get_level(BTN_RIGHT_PIN),
            (bool) gpio_get_level(BTN_LEFT_PIN),
        };

        while (true)
        {
            for (int i = 0; i < 2; i++)
            {
                if (btn_states_now[i] && !btn_states[i])
                {
                    last_press[i] = xTaskGetTickCount();
                }
                else if (!btn_states_now[i] && btn_states[i])
                {
                    if (callbacks.onReleased[i]) callbacks.onReleased[i]();
                    long_pressed[i] = false;
                }
                else
                {
                    int32_t press_duration = xTaskGetTickCount() - (int32_t)last_press[i]; // casting to int32_t to avoid overflow
                    if (btn_states[i] && press_duration > BTN_LONG_PRESS_MS / portTICK_PERIOD_MS)
                    {
                        if (callbacks.onLongPressed[i]) callbacks.onLongPressed[i]();
                        last_press[i] = xTaskGetTickCount() + BTN_LONG_PRESS_MS*2 / portTICK_PERIOD_MS;
                        long_pressed[i] = true;
                    } else if (btn_states[i] && !btn_states_now[i] && !long_pressed[i])
                    {
                        if (callbacks.onPressed[i]) callbacks.onPressed[i]();
                    }
                }
            }

            for (int i = 0; i < 2; i++)
            {
                btn_states[i] = btn_states_now[i];
                long_pressed[i] &= btn_states[i];
            }

            vTaskDelay(BTN_POLL_INT_MS / portTICK_PERIOD_MS);
        }
    }

    Error Init()
    {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << BTN_LEFT_PIN) | (1ULL << BTN_RIGHT_PIN);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        
        if (gpio_config(&io_conf) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "GPIO: Failed to configure GPIO pins");
            return Error::Unknown;
        }

        if (xTaskCreate(update_task, "Buttons::update_task", 2048, nullptr, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
        {
            Log::Add(Log::Level::Error, "GPIO: Failed to create GPIO update task");
            return Error::Unknown;
        }

        return Error::Ok;
    }

    Error SetCallbacks(const CallbackSet& cb)
    {
        callbacks = cb;
        return Error::Ok;
    }
}