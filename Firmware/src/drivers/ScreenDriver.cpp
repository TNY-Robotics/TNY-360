#include "drivers/ScreenDriver.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"
#include "drivers/ScreenDriver.Error.hpp"
#include "esp_lcd_panel_sh1106.h"
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

namespace ScreenDriver
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

    Status Init()
    {
        if (Status err = I2C::Init(); err != Status::Ok)
        {
            Error::RegisterErrorEvent(ErrorEventI2CInitFailed());
            return err;
        }

        esp_lcd_panel_io_handle_t io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t io_config = ESP_SH1106_DEFAULT_IO_CONFIG;
        if (esp_err_t err = esp_lcd_new_panel_io_i2c(I2C::handle_secondary, &io_config, &io_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Couldn't create panel IO");
            
            return Status::Failure;
        }

        esp_lcd_panel_dev_config_t panel_config = {
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
            .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
            .bits_per_pixel = SH1106_PIXELS_PER_BYTE / 8,
            .reset_gpio_num = GPIO_NUM_NC,
            .vendor_config = NULL,
            .flags = {
                .reset_active_high = false,
            },
        };
        if (esp_err_t err = esp_lcd_new_panel_sh1106(io_handle, &panel_config, &panel_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Couldn't create sh1106 panel");
            Error::RegisterErrorEvent(ErrorEventPanelCreateFailed(err));
            return Status::Failure;
        }
        if (esp_err_t err = esp_lcd_panel_reset(panel_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Couln't reset panel");
            Error::RegisterErrorEvent(ErrorEventPanelResetFailed(err));
            return Status::Failure;
        }
        if (esp_err_t err = esp_lcd_panel_init(panel_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Couldn't init panel");
            Error::RegisterErrorEvent(ErrorEventPanelInitFailed(err));
            return Status::Failure;
        }

        if (Status err = Clear(); err != Status::Ok) return err;
        if (Status err = Upload(); err != Status::Ok) return err;

        if (esp_err_t err = esp_lcd_panel_disp_on_off(panel_handle, true); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Couldn't turn the display on");
            Error::RegisterErrorEvent(ErrorEventPanelDisplayOnFailed(err));
            return Status::Failure;
        }
        
        info = {
            .data = screen_data,
            .width = SH1106_WIDTH,
            .height = SH1106_HEIGHT,
        };

        return Status::Ok;
    }

    Status Deinit()
    {
        if (esp_err_t err = esp_lcd_panel_del(panel_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Couldn't delete panel");
            Error::RegisterErrorEvent(ErrorEventPanelDeleteFailed(err));
            return Status::Failure;
        }
        panel_handle = NULL;
        return Status::Ok;
    }

    Status Clear()
    {
        memset(screen_data, 0, SH1106_HEIGHT * SH1106_WIDTH);
        memset(buffer_data, 0, SH1106_BUFFER_SIZE);
        return Status::Ok;
    }

    Status Upload()
    {
        screen_to_buffer(&info, buffer_data);
        esp_err_t err = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, SH1106_WIDTH, SH1106_HEIGHT, buffer_data);
        if (err != ESP_OK) {
            LOG_ERROR(TAG, "Couldn't draw bitmap on panel");
            Error::RegisterErrorEvent(ErrorEventUploadFailed(err));
            return Status::Failure;
        }
        return Status::Ok;
    }
}
