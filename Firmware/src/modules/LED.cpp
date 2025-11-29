#include "modules/LED.hpp"
#include "modules/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <driver/rmt_tx.h>
#include <esp_log.h>
#include <cmath>
#include <memory.h>

namespace LED
{
    static constexpr gpio_num_t LED_GPIO = GPIO_NUM_48; // GPIO pin for LED data
    static constexpr int RMT_RESOLUTION_HZ = 10'000'000; // RMT resolution in Hz

    static const char* TAG = "LED";

    static constexpr uint16_t WS2812_T0H_TICKS = 4;  // 0.4us
    static constexpr uint16_t WS2812_T0L_TICKS = 9;  // 0.9us (Total 1.3us for '0')
    static constexpr uint16_t WS2812_T1H_TICKS = 8;  // 0.8us
    static constexpr uint16_t WS2812_T1L_TICKS = 5;  // 0.5us (Total 1.3us for '1')

    static uint8_t target_pixels[LED_COUNT * 3] = {0}; // target color to reach after all transitions
    static uint8_t last_pixels[LED_COUNT * 3] = {0}; // current color shown on the strip (last one sent)
    static uint8_t adder_pixels[LED_COUNT * 3] = {0}; // per update increment to reach target in desired time

    static rmt_channel_handle_t rmt_tx_channel = NULL;
    static rmt_encoder_handle_t rmt_bytes_encoder = NULL;
    static rmt_transmit_config_t transmit_config = { .loop_count = 0 }; // Transmit once by default

    static uint8_t rmt_buffer[2][LED_COUNT * 3] = {0}; 
    static int current_buffer_idx = 0;

    static uint8_t UPDATE_TASK_INTERVAL_MS = 50; // Update task interval in milliseconds

    void _update_task(void* param); // forward declaration

    Error Init()
    {
        ESP_LOGI(TAG, "Create RMT TX channel for WS2812");

        // setup led control using RMT
        rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = LED_GPIO,
            .clk_src = RMT_CLK_SRC_DEFAULT, // Use default clock source (usually APB CLK)
            .resolution_hz = RMT_RESOLUTION_HZ,
            .mem_block_symbols = 64, // ESP32/S3/S2 typically have 64 symbols per block.
                                    // C3/C6/H2 might have 48. Driver can chain blocks if needed.
                                    // Set to a value >= (bytes_to_transmit * 8) if you want to ensure one transaction.
                                    // Or let driver manage it.
            .trans_queue_depth = 2, // Allows queuing of transactions
            .flags = {
                .invert_out = false, // WS2812 data is not inverted
            }
        };

        ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &rmt_tx_channel));

        ESP_LOGI(TAG, "Install RMT bytes encoder for WS2812");
        rmt_bytes_encoder_config_t bytes_encoder_config = {
            .bit0 = {
                .duration0 = WS2812_T0H_TICKS,
                .level0 = 1,
                .duration1 = WS2812_T0L_TICKS,
                .level1 = 0,
            },
            .bit1 = {
                .duration0 = WS2812_T1H_TICKS,
                .level0 = 1,
                .duration1 = WS2812_T1L_TICKS,
                .level1 = 0,
            },
            .flags = {
                .msb_first = 1 // WS2812 data is MSB first
            }
        };
        ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytes_encoder_config, &rmt_bytes_encoder));

        ESP_LOGI(TAG, "Enable RMT TX channel");
        ESP_ERROR_CHECK(rmt_enable(rmt_tx_channel));

        // launch background task for led update at fixed interval
        ESP_LOGI(TAG, "Launching update task");
        TaskHandle_t task_handle;
        xTaskCreate(_update_task, "updateLEDs", 4096, nullptr, 5, &task_handle);
        
        return Error::Ok;
    }

    Error SetColor(Id id, Color color)
    {
        if (id >= LED_COUNT) {
            return Error::InvalidParameters;
        }

        target_pixels[id * 3 + 0] = color.g; // WS2812 expects GRB order
        target_pixels[id * 3 + 1] = color.r;
        target_pixels[id * 3 + 2] = color.b;
        last_pixels[id * 3 + 0] = color.g; // apply immediately
        last_pixels[id * 3 + 1] = color.r;
        last_pixels[id * 3 + 2] = color.b;
        return Error::Ok;
    }

    Error SetColor(Id id, Color color, float duration_s)
    {
        if (id >= LED_COUNT) {
            return Error::InvalidParameters;
        }

        const float updates_count = duration_s * 1000.0f / static_cast<float>(UPDATE_TASK_INTERVAL_MS);
        if (updates_count <= 0.0f) {
            return SetColor(id, color); // apply immediately
        }

        target_pixels[id * 3 + 0] = color.g; // WS2812 expects GRB order
        target_pixels[id * 3 + 1] = color.r;
        target_pixels[id * 3 + 2] = color.b;
        
        // calculate adder for each color component
        const int16_t g_diff = std::abs(static_cast<int16_t>(target_pixels[id * 3 + 0]) - static_cast<int16_t>(last_pixels[id * 3 + 0]));
        const int16_t r_diff = std::abs(static_cast<int16_t>(target_pixels[id * 3 + 1]) - static_cast<int16_t>(last_pixels[id * 3 + 1]));
        const int16_t b_diff = std::abs(static_cast<int16_t>(target_pixels[id * 3 + 2]) - static_cast<int16_t>(last_pixels[id * 3 + 2]));

        adder_pixels[id * 3 + 0] = static_cast<uint8_t>(std::ceil(static_cast<float>(g_diff) / updates_count));
        adder_pixels[id * 3 + 1] = static_cast<uint8_t>(std::ceil(static_cast<float>(r_diff) / updates_count));
        adder_pixels[id * 3 + 2] = static_cast<uint8_t>(std::ceil(static_cast<float>(b_diff) / updates_count));

        return Error::Ok;
    }

    Error SetColors(const Color colors[])
    {
        for (Id i = 0; i < LED_COUNT; ++i) {
            SetColor(i, colors[i]);
        }
        return Error::Ok;
    }

    Error SetColors(const Color colors[], const float durations_s[])
    {
        for (Id i = 0; i < LED_COUNT; ++i) {
            SetColor(i, colors[i], durations_s[i]);
        }
        return Error::Ok;
    }

    Error SetColors(const Id ids[], const Color colors[], uint8_t count)
    {
        for (uint8_t i = 0; i < count; ++i) {
            SetColor(ids[i], colors[i]);
        }
        return Error::Ok;
    }

    Error SetColors(const Id ids[], const Color colors[], const float durations_s[], uint8_t count)
    {
        for (uint8_t i = 0; i < count; ++i) {
            SetColor(ids[i], colors[i], durations_s[i]);
        }
        return Error::Ok;
    }

    Error GetColor(Id id, Color* outColor)
    {
        if (id >= LED_COUNT || outColor == nullptr) {
            return Error::InvalidParameters;
        }
        outColor->g = target_pixels[id * 3 + 0]; // WS2812 stores in GRB order
        outColor->r = target_pixels[id * 3 + 1];
        outColor->b = target_pixels[id * 3 + 2];
        return Error::Ok;
    }

    Error GetColors(Color* outColors)
    {
        if (outColors == nullptr) {
            return Error::InvalidParameters;
        }
        for (Id i = 0; i < LED_COUNT; ++i) {
            outColors[i].g = target_pixels[i * 3 + 0]; // WS2812 stores in GRB order
            outColors[i].r = target_pixels[i * 3 + 1];
            outColors[i].b = target_pixels[i * 3 + 2];
        }
        return Error::Ok;
    }

    Error GetColors(const Id ids[], Color* outColors, uint8_t count)
    {
        if (ids == nullptr || outColors == nullptr) {
            return Error::InvalidParameters;
        }
        for (uint8_t i = 0; i < count; ++i) {
            Id id = ids[i];
            if (id >= LED_COUNT) {
                return Error::InvalidParameters;
            }
            outColors[i].g = target_pixels[id * 3 + 0]; // WS2812 stores in GRB order
            outColors[i].r = target_pixels[id * 3 + 1];
            outColors[i].b = target_pixels[id * 3 + 2];
        }
        return Error::Ok;
    }

    void LoopErrorCode(uint8_t errCode)
    {
#if DEBUG_MODE == 1
        return; // skip in debug mode to avoid blocking
#endif
        // turn everything off first
        for (Id i = 0; i < LED_COUNT; ++i) {
            SetColor(i, {0, 0, 0}, 0.1f);
        }

        while (1)
        {
            // turn off first led and wait for 2 seconds
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            // turn first led on during 2 seconds (start of error code display)
            SetColor(0, {255, 0, 0}, 0.1f);
            vTaskDelay(pdMS_TO_TICKS(2000));

            // turn off first led for 500ms before displaying error code
            SetColor(0, {0, 0, 0}, 0.1f);
            vTaskDelay(pdMS_TO_TICKS(500));

            // Show error code in red blinks (500ms per byte)
            for (uint8_t i = 0; i < 8; ++i) {
                if (errCode & (1 << (7 - i))) {
                    SetColor(0, {255, 0, 0}, 0.1f); // Red for '1'
                } else {
                    SetColor(0, {0, 0, 255}, 0.1f); // Blue for '0'
                }
                vTaskDelay(pdMS_TO_TICKS(500));
                SetColor(0, {0, 0, 0}, 0.1f); // Off between bits
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }

    void _update_task(void* param)
    {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t xFrequency = pdMS_TO_TICKS(UPDATE_TASK_INTERVAL_MS);

        while (true) {
            for (size_t i = 0; i < LED_COUNT * 3; ++i) {
                if (last_pixels[i] != target_pixels[i]) {
                    if (last_pixels[i] < target_pixels[i]) {
                        last_pixels[i] += std::min(adder_pixels[i], static_cast<uint8_t>(target_pixels[i] - last_pixels[i]));
                    } else {
                        last_pixels[i] -= std::min(adder_pixels[i], static_cast<uint8_t>(last_pixels[i] - target_pixels[i]));
                    }
                }
            }

            ESP_ERROR_CHECK(rmt_tx_wait_all_done(rmt_tx_channel, portMAX_DELAY));

            memcpy(rmt_buffer[current_buffer_idx], last_pixels, sizeof(last_pixels));

            esp_err_t err = rmt_transmit(rmt_tx_channel, rmt_bytes_encoder, rmt_buffer[current_buffer_idx], sizeof(last_pixels), &transmit_config);
            if (err != ESP_OK) {
                ESP_LOGW(TAG, "RMT TX transmit failed: %s", esp_err_to_name(err));
            }
            
            current_buffer_idx = (current_buffer_idx + 1) % 2;
            xTaskDelayUntil(&xLastWakeTime, xFrequency);
        }
    }
}