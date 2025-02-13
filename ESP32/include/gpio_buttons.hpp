#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

esp_err_t gpio_buttons_init(gpio_num_t left, gpio_num_t right);
esp_err_t gpio_buttons_scan();

void gpio_buttons_set_cb(void(*onLeftPressed)(), void(*onLeftLongPressed)(), void(*onRightPressed)(), void(*onRightLongPressed)());