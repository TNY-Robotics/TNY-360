#pragma once
#include "ui/Screen.hpp"
#include "esp_lcd_panel_ssd1306.h"
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

class ScreenSSD : public Screen
{
public:
    static constexpr uint16_t SSD1306_PAGE_HEIGHT = 8;
    static constexpr uint16_t SSD1306_WIDTH = 128;
    static constexpr uint16_t SSD1306_HEIGHT = 64;
    static constexpr uint16_t SSD1306_BUFFER_SIZE = SSD1306_WIDTH * SSD1306_HEIGHT / 8;

    ScreenSSD();

    Status init() override;

    Status deinit() override;

    Status clear() override;

    Status upload() override;

private:
    esp_lcd_panel_handle_t panel_handle = NULL;
    bool* screen_data;
    uint8_t* buffer_data;

    /* Convert a screen buffer data to a lcd-format image buffer (SSD1309)
    * @param screen_info: the screen buffer information, see screen_info_t
    * @param buffer_info: the buffer information, see buffer_info_t
    */
    void screen_to_buffer(uint8_t* buffer_data);
};