#include "ui/ScreenSSD.hpp"
#include "ui/Screen.Error.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"

void ScreenSSD::screen_to_buffer(uint8_t* buffer_data)
{
    uint16_t page_count = info.height / SSD1306_PAGE_HEIGHT;
    for (uint16_t i = 0; i < page_count; i++)
    {
        for (uint16_t j = 0; j < info.width; j++)
        {
            uint8_t byte = 0;
            for (uint16_t k = 0; k < SSD1306_PAGE_HEIGHT; k++)
            {
                byte |= (info.data[(i * SSD1306_PAGE_HEIGHT + k) * info.width + j] ? 1 : 0) << k;
            }
            buffer_data[i * info.width + j] = byte;
        }
    }
}

ScreenSSD::ScreenSSD()
{
    setInstance(this);
    screen_data = new bool[SSD1306_HEIGHT * SSD1306_WIDTH];
    buffer_data = new uint8_t[SSD1306_BUFFER_SIZE];
}

Status ScreenSSD::init()
{
    if (Status err = I2C::Init(); err != Status::Ok)
    {
        Error::RegisterErrorEvent(ErrorEventI2CInitFailed());
        return err;
    }

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = 0x3C,
        .scl_speed_hz = 400 * 1000,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .flags = {
            .dc_low_on_data = false,
            .disable_control_phase = false,
        },
    };
    if (esp_err_t err = esp_lcd_new_panel_io_i2c(I2C::handle_secondary, &io_config, &io_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Couldn't create panel IO");
        
        return Status::Failure;
    }

    esp_lcd_panel_dev_config_t panel_config = {
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 1,
        .reset_gpio_num = GPIO_NUM_NC,
        .vendor_config = NULL,
        .flags = {
            .reset_active_high = false,
        },
    };
    if (esp_err_t err = esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Couldn't create ssd1306 panel");
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
    // Invert image top/down
    if (esp_err_t err = esp_lcd_panel_mirror(panel_handle, true, true); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Couldn't invert panel colors");
        // Error::RegisterErrorEvent(ErrorEventPanelMirrorFailed(err));
        return Status::Failure;
    }

    if (Status err = clear(); err != Status::Ok) return err;
    if (Status err = upload(); err != Status::Ok) return err;

    if (esp_err_t err = esp_lcd_panel_disp_on_off(panel_handle, true); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Couldn't turn the display on");
        Error::RegisterErrorEvent(ErrorEventPanelDisplayOnFailed(err));
        return Status::Failure;
    }
    
    info = {
        .data = screen_data,
        .width = SSD1306_WIDTH,
        .height = SSD1306_HEIGHT,
    };

    return Status::Ok;
}

Status ScreenSSD::deinit()
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

Status ScreenSSD::clear()
{
    memset(screen_data, 0, SSD1306_HEIGHT * SSD1306_WIDTH);
    memset(buffer_data, 0, SSD1306_BUFFER_SIZE);
    return Status::Ok;
}

Status ScreenSSD::upload()
{
    screen_to_buffer(buffer_data);
    esp_err_t err = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, SSD1306_WIDTH, SSD1306_HEIGHT, buffer_data);
    if (err != ESP_OK) {
        LOG_ERROR(TAG, "Couldn't draw bitmap on panel");
        Error::RegisterErrorEvent(ErrorEventUploadFailed(err));
        return Status::Failure;
    }
    return Status::Ok;
}
