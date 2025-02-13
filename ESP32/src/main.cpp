#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_buttons.hpp"
#include "draw.hpp"
#include "screen.hpp"
#include "menulist.hpp"

#define BTN_LEFT_GPIO GPIO_NUM_45
#define BTN_RIGHT_GPIO GPIO_NUM_46

#define I2C_SDA_GPIO GPIO_NUM_47
#define I2C_SCL_GPIO GPIO_NUM_21
#define I2C_HOST I2C_NUM_0

#ifdef __cplusplus
extern "C"
#endif
void app_main(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_HOST,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
            .allow_pd = false,
        },
    };
    i2c_master_bus_handle_t i2c_bus_handle = NULL;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    screen_init(&i2c_bus_handle);
    gpio_buttons_init(BTN_LEFT_GPIO, BTN_RIGHT_GPIO);
    gpio_buttons_set_cb(
        []() { get_current_menu()->onLeftPressed(); },
        []() { get_current_menu()->onLeftLongPressed(); },
        []() { get_current_menu()->onRightPressed(); },
        []() { get_current_menu()->onRightLongPressed(); }
    );
    menu_init();
    
    while (1)
    {
        gpio_buttons_scan();

        get_current_menu()->update();
        get_current_menu()->render();

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}