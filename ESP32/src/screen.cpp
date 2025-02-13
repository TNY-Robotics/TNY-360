#include "screen.hpp"
#include <memory.h>
#include "esp_lcd_io_i2c.h"
#include "esp_lcd_panel_sh1106.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#define SH1106_PAGE_HEIGHT 8

bool screen_data[SH1106_HEIGHT * SH1106_WIDTH];
uint8_t buffer_data[SH1106_BUFFER_SIZE];
esp_lcd_panel_handle_t panel_handle = NULL;
    
screen_info_t screen_info = {
    .data = screen_data,
    .width = SH1106_WIDTH,
    .height = SH1106_HEIGHT,
};

/* Convert a screen buffer data to a lcd-format image buffer (e.g. SH1106)
 * @param screen_info: the screen buffer information, see screen_info_t
 * @param buffer_info: the buffer information, see buffer_info_t
 */
void screen_to_buffer(screen_info_t* screen_info, uint8_t* buffer_data)
{
    uint16_t page_count = screen_info->height / SH1106_PAGE_HEIGHT;
    for (uint16_t i = 0; i < page_count; i++)
    {
        for (uint16_t j = 0; j < screen_info->width; j++)
        {
            uint8_t byte = 0;
            for (uint16_t k = 0; k < SH1106_PAGE_HEIGHT; k++)
            {
                byte |= (screen_info->data[(i * SH1106_PAGE_HEIGHT + k) * screen_info->width + j] ? 1 : 0) << k;
            }
            buffer_data[i * screen_info->width + j] = byte;
        }
    }
}

void screen_init(const i2c_master_bus_handle_t* i2c_bus_handle)
{
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = ESP_SH1106_DEFAULT_IO_CONFIG;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(*i2c_bus_handle, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = SH1106_PIXELS_PER_BYTE / 8,
        .flags = {
            .reset_active_high = false,
        },
        .vendor_config = NULL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // Clear all screen data and upload it
    screen_clear();
    screen_upload();
}

void screen_clear()
{
    memset(screen_data, 0, SH1106_HEIGHT * SH1106_WIDTH);
    memset(buffer_data, 0, SH1106_BUFFER_SIZE);
}

void screen_upload()
{
    screen_to_buffer(&screen_info, buffer_data);
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, SH1106_WIDTH, SH1106_HEIGHT, buffer_data));
}

screen_info_t* get_screen_info()
{
    static screen_info_t screen_info = {
        .data = screen_data,
        .width = SH1106_WIDTH,
        .height = SH1106_HEIGHT,
    };
    return &screen_info;
}
