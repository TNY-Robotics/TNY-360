#include "gpio_buttons.hpp"

#define BTN_LONG_PRESS_MS 500

bool btn_states[2] = {false, false};
bool long_pressed[2] = {0, 0};
TickType_t last_press[2] = {0, 0};
void(*onPressed[2])() = {NULL, NULL};
void(*onLongPressed[2])() = {NULL, NULL};

gpio_num_t left_btn_gpio = GPIO_NUM_NC;
gpio_num_t right_btn_gpio = GPIO_NUM_NC;

esp_err_t gpio_buttons_init(gpio_num_t left, gpio_num_t right)
{
    left_btn_gpio = left;
    right_btn_gpio = right;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << left) | (1ULL << right);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // should have external pull-down
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    return gpio_config(&io_conf);
}

esp_err_t gpio_buttons_scan()
{
    bool btn_states_now[2] = {
        (bool) gpio_get_level(right_btn_gpio),
        (bool) gpio_get_level(left_btn_gpio),
    };
    
    for (int i = 0; i < 2; i++)
    {
        if (btn_states_now[i] && !btn_states[i])
        {
            last_press[i] = xTaskGetTickCount();
        } else
        {
            int64_t press_duration = xTaskGetTickCount() - (int64_t)last_press[i]; // casting to singed int64_t to avoid overflow
            if (btn_states[i] && press_duration > BTN_LONG_PRESS_MS / portTICK_PERIOD_MS)
            {
                if (onLongPressed[i]) onLongPressed[i]();
                last_press[i] = xTaskGetTickCount() + BTN_LONG_PRESS_MS*2 / portTICK_PERIOD_MS;
                long_pressed[i] = true;
            } else if (btn_states[i] && !btn_states_now[i] && !long_pressed[i])
            {
                if (onPressed[i]) onPressed[i]();
            }
        }
    }

    for (int i = 0; i < 2; i++)
    {
        btn_states[i] = btn_states_now[i];
        long_pressed[i] &= btn_states[i];
    }

    return ESP_OK;
}

void gpio_buttons_set_cb(void(*onLeftPressed)(), void(*onLeftLongPressed)(), void(*onRightPressed)(), void(*onRightLongPressed)())
{
    onPressed[0] = onRightPressed;
    onLongPressed[0] = onRightLongPressed;
    onPressed[1] = onLeftPressed;
    onLongPressed[1] = onLeftLongPressed;
}