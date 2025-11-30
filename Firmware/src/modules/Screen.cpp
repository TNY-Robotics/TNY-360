#include "modules/Screen.hpp"
#include "modules/I2C.hpp"
#include "esp_lcd_panel_sh1106.h"
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include "font8x8_basic.h"

char (*screen_font)[8] = font8x8_basic;

namespace Screen
{
    Info info;

    constexpr uint16_t SH1106_PAGE_HEIGHT = 8;

    esp_lcd_panel_handle_t panel_handle = NULL;
    
    bool screen_data[SH1106_HEIGHT * SH1106_WIDTH];
    uint8_t buffer_data[SH1106_BUFFER_SIZE];

    /* Convert a screen buffer data to a lcd-format image buffer (e.g. SH1106)
    * @param screen_info: the screen buffer information, see screen_info_t
    * @param buffer_info: the buffer information, see buffer_info_t
    */
    void screen_to_buffer(Info* screen_info, uint8_t* buffer_data)
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

    Error Init() {
        esp_lcd_panel_io_handle_t io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t io_config = ESP_SH1106_DEFAULT_IO_CONFIG;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(I2C::handle_secondary, &io_config, &io_handle));

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
        
        info = {
            .data = screen_data,
            .width = SH1106_WIDTH,
            .height = SH1106_HEIGHT,
        };

        if (Error err = Clear(); err != Error::Ok) return err;
        if (Error err = Upload(); err != Error::Ok) return err;

        return Error::Ok;
    }

    Error Clear()
    {
        memset(screen_data, 0, SH1106_HEIGHT * SH1106_WIDTH);
        memset(buffer_data, 0, SH1106_BUFFER_SIZE);
        return Error::Ok;
    }

    Error Upload()
    {
        screen_to_buffer(&info, buffer_data);
        esp_err_t err = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, SH1106_WIDTH, SH1106_HEIGHT, buffer_data);
        if (err != ESP_OK) {
            return Error::Unknown;
        }
        return Error::Ok;
    }
}

namespace Draw
{
    uint16_t GetTextWidth(const char* text)
    {
        uint16_t width = 0;
        while (*text)
        {
            width += 8;
            text++;
        }
        return width;
    }
}