#include "ui/Button.hpp"
#include "common/Log.hpp"
#include "settings/Settings.hpp"
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

namespace Button
{
    gpio_num_t gpio_pin_left = GPIO_NUM_NC;
    gpio_num_t gpio_pin_right = GPIO_NUM_NC;
    int long_press_duration_ms = -1;
    int polling_interval_ms = -1;

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
                (bool) gpio_get_level(gpio_pin_left),
                (bool) gpio_get_level(gpio_pin_right),
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
                    if (btn_states[i] && press_duration > pdMS_TO_TICKS(long_press_duration_ms) && !long_pressed[i])
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

            vTaskDelay(pdMS_TO_TICKS(polling_interval_ms));
        }
    }

    Status Init()
    {
        LOG_SCOPE(TAG, "Button::Init");
        if (initialized)
        {
            LOG_DEBUG(TAG, "Already initialized");
            return Status::Ok;
        }

        gpio_pin_left = static_cast<gpio_num_t>(Settings::GetConfig().buttons.gpio_left);
        gpio_pin_right = static_cast<gpio_num_t>(Settings::GetConfig().buttons.gpio_right);
        long_press_duration_ms = Settings::GetConfig().buttons.long_press_duration_ms;
        polling_interval_ms = Settings::GetConfig().buttons.polling_interval_ms;

        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << gpio_pin_left) | (1ULL << gpio_pin_right);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        
        if (gpio_config(&io_conf) != ESP_OK)
        {
            LOG_ERROR(TAG, "Buttons: Failed to configure GPIO pins");
            // ErrorHandle(ErrorStruct::ButtonsInitFailed);
            return Status::Unknown;
        }

        if (xTaskCreatePinnedToCore(update_task, "Buttons::update_task", 8192, nullptr, tskIDLE_PRIORITY + 1, nullptr, CORE_BRAIN) != pdPASS)
        {
            LOG_ERROR(TAG, "Buttons: Failed to create Buttons update task");
            // ErrorHandle(ErrorStruct::ButtonsInitFailed);
            return Status::Unknown;
        }

        LOG_DEBUG(TAG, "Buttons initialized successfully");

        initialized = true;
        return Status::Ok;
    }

    Status SetCallbacks(const CallbackSet& cb)
    {
        callbacks = cb;
        return Status::Ok;
    }
}