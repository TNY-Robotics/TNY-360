#pragma once
#include "ui/Screen.hpp"
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include "esp_lcd_panel_sh1106.h"

class ScreenSH : public Screen
{
public:
    static constexpr uint16_t SH1106_PAGE_HEIGHT = 8;

    ScreenSH();

    Status init() override;

    Status deinit() override;

    Status clear() override;

    Status upload() override;

private:
    bool* screen_data;
    uint8_t* buffer_data;

    esp_lcd_panel_handle_t panel_handle = NULL;

    /* Convert a screen buffer data to a lcd-format image buffer (SH1106)
    * @param screen_info: the screen buffer information, see screen_info_t
    * @param buffer_info: the buffer information, see buffer_info_t
    */
    void screen_to_buffer(uint8_t* buffer_data);
};