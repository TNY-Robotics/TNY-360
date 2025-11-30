#include "modules/Button.hpp"
#include "modules/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

namespace Button
{
    bool btn_states[2] = {false, false};
    bool long_pressed[2] = {0, 0};
    TickType_t last_press[2] = {0, 0};

    CallbackSet callbacks = { {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr} };

    bool initialized = false;

    void update_task(void* pvParams)
    {
        while (true)
        {
            bool btn_states_now[2] = {
                (bool) gpio_get_level(BTN_LEFT_PIN),
                (bool) gpio_get_level(BTN_RIGHT_PIN),
            };
            
            for (int i = 0; i < 2; i++)
            {
                if (btn_states_now[i] && !btn_states[i])
                {
                    last_press[i] = xTaskGetTickCount();
                    if (callbacks.onPressed[i]) callbacks.onPressed[i]();
                }
                else if (!btn_states_now[i] && btn_states[i])
                {
                    if (callbacks.onReleased[i]) callbacks.onReleased[i]();
                    long_pressed[i] = false;
                }
                else
                {
                    int32_t press_duration = xTaskGetTickCount() - (int32_t)last_press[i]; // casting to int32_t to avoid overflow
                    if (btn_states[i] && press_duration > pdMS_TO_TICKS(BTN_LONG_PRESS_MS) && !long_pressed[i])
                    {
                        if (callbacks.onLongPressed[i]) callbacks.onLongPressed[i]();
                        long_pressed[i] = true;
                    }
                }
            }

            for (int i = 0; i < 2; i++)
            {
                btn_states[i] = btn_states_now[i];
                long_pressed[i] &= btn_states[i];
            }

            vTaskDelay(pdMS_TO_TICKS(BTN_POLL_INT_MS));
        }
    }

    Error Init()
    {
        if (initialized) return Error::Ok;

        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << BTN_LEFT_PIN) | (1ULL << BTN_RIGHT_PIN);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        
        if (gpio_config(&io_conf) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Buttons: Failed to configure GPIO pins");
            return Error::Unknown;
        }

        if (xTaskCreate(update_task, "Buttons::update_task", 8192, nullptr, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
        {
            Log::Add(Log::Level::Error, "Buttons: Failed to create Buttons update task");
            return Error::Unknown;
        }

        initialized = true;

        return Error::Ok;
    }

    Error SetCallbacks(const CallbackSet& cb)
    {
        callbacks = cb;
        return Error::Ok;
    }
}