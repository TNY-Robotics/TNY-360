#pragma once
#include "settings/config.hpp"

constexpr RobotConfig CONFIG_V2 = {
    .i2c = {
        .primary = {
            .gpio_sda = 5,
            .gpio_scl = 4,
        },
        .secondary = {
            .gpio_sda = 7,
            .gpio_scl = 6,
        },
    },
    .audio = {
        .speakerType = AudioSpeakerType::SPEAKER_I2S,
        .i2s = {
            .gpio_clk = 2,
            .gpio_ws = 1,
            .gpio_data = 42,
        },
        .volume = 0.5f,
        .sampleRate = 22050,
        .nb_providers = 4,
    },
    .camera = {
        .gpio = {
            .pin_pwdn = -1,
            .pin_reset = -1,
            .pin_xclk = 14,
            .pin_sccb_sda = -1,
            .pin_sccb_scl = -1,
            .pin_d7 = 21,
            .pin_d6 = 13,
            .pin_d5 = 12,
            .pin_d4 = 10,
            .pin_d3 = 46,
            .pin_d2 = 20,
            .pin_d1 = 3,
            .pin_d0 = 9,
            .pin_vsync = 48,
            .pin_href = 47,
            .pin_pclk = 11,
        },
        .xclk_freq_hz = 20000000,
    },
    .screen = {
        .type = ScreenType::SCREEN_SSD1309,
    },
    .buttons = {
        .gpio_left = 41,
        .gpio_right = 40,
        .polling_interval_ms = 50,
        .long_press_duration_ms = 250,
    },
    .analog = {
        .gpio_select_1 = 15,
        .gpio_select_2 = 16,
        .gpio_select_3 = 17,
        .gpio_select_4 = 18,
        .channel = 7,
    },
    .imu = {
        .type = IMUType::IMU_LSM6DS3,
    },
};