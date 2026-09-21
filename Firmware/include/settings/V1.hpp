#pragma once
#include "settings/config.hpp"

constexpr RobotConfig CONFIG_V1 = {
    .i2c = {
        .primary = {
            .gpio_sda = 21,
            .gpio_scl = 47,
        },
        .secondary = {
            .gpio_sda = 9,
            .gpio_scl = 48,
        },
    },
    .audio = {
        .speakerType = AudioSpeakerType::SPEAKER_NONE,
        .pdm = {
            .gpio_out = 1,
        },
        .volume = 0.5f,
        .sampleRate = 22500,
        .nb_providers = 4,
    },
    .camera = {
        .gpio = {
            .pin_pwdn = -1,
            .pin_reset = -1,
            .pin_xclk = 8,
            .pin_sccb_sda = -1,
            .pin_sccb_scl = -1,
            .pin_d7 = 19,
            .pin_d6 = 18,
            .pin_d5 = 17,
            .pin_d4 = 15,
            .pin_d3 = 6,
            .pin_d2 = 4,
            .pin_d1 = 5,
            .pin_d0 = 7,
            .pin_vsync = 3,
            .pin_href = 20,
            .pin_pclk = 16,
        },
        .xclk_freq_hz = 20000000,
    },
    .screen = {
        .type = ScreenType::SCREEN_SH1106,
    },
    .buttons = {
        .gpio_left = 11,
        .gpio_right = 10,
        .polling_interval_ms = 50,
        .long_press_duration_ms = 300,
    },
    .analog = {
        .gpio_select_1 = 39,
        .gpio_select_2 = 40,
        .gpio_select_3 = 41,
        .gpio_select_4 = 42,
        .channel = 1,
    },
    .imu = {
        .type = IMUType::IMU_MPU6050,
    },
};