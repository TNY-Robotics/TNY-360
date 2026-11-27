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
        .speaker = {
            .type = SpeakerType::SPEAKER_PDM,
            .pdm = {
                .gpio_out = 1,
            },
            .volume = 0.5f,
            .sampleRate = 22500,
            .nb_providers = 4,
        },
        .microphone = {
            .type = MicrophoneType::MICROPHONE_I2S,
            .i2s = {
                .gpio_clk = 14,
                .gpio_ws = 12,
                .gpio_data = 13,
            },
            .gain = 10,
            .sampleRate = 16000,
        },
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
        .refresh_rate = 30.0f,
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
        .ema_alpha = 0.1f,
    },
    .imu = {
        .type = IMUType::IMU_MPU6050,
        .i2c_address = 0x68,
        .i2c_clock_speed = 400000,
    },
    .kinematics = {
        .leg_thigh_length = 0.1f,
        .leg_calf_length = 0.1f,
        .hip_offset = 0.034f,
        .hip_pos_x = 0.086f,
        .hip_pos_y = 0.052f,
    },
    .body {
        .ear_r = {
            .motor_driver_index = 0,
            .analog_reader_index = 0, // no feedback
            .min_angle_rad = -0.785f, // -45d
            .max_angle_rad = 0.785f, // 45d
            .is_inverted = true,
            .has_feedback = false,
            .has_mechanical_stops = false,
            .min_stop_rad = 0.000f, // no mechanical stop
            .max_stop_rad = 0.000f, // no mechanical stop
        },
        .ear_l = {
            .motor_driver_index = 1,
            .analog_reader_index = 0, // no feedback
            .min_angle_rad = -0.785f, // -45d
            .max_angle_rad = 0.785f, // 45d
            .is_inverted = false,
            .has_feedback = false,
            .has_mechanical_stops = false,
            .min_stop_rad = 0.000f, // no mechanical stop
            .max_stop_rad = 0.000f, // no mechanical stop
        },
        .leg_fl = {
            .hip_roll = {
                .motor_driver_index = 2,
                .analog_reader_index = 0,
                .min_angle_rad = -0.785f, // -45d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = false,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.785f,
                .max_stop_rad = 2.356f,
            },
            .hip_pitch = {
                .motor_driver_index = 3,
                .analog_reader_index = 1,
                .min_angle_rad = -2.356f, // -135d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = true,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.000f,
                .max_stop_rad = 3.142f,
            },
            .knee_pitch = {
                .motor_driver_index = 4,
                .analog_reader_index = 2,
                .min_angle_rad = 0.0f, // 0d
                .max_angle_rad = 2.618f, // 150d
                .is_inverted = false,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.262f,
                .max_stop_rad = 2.880f,
            },
            .contact_sensor_index = 3,
            .has_y_inverted = true,
        },
        .leg_bl = {
            .hip_roll = {
                .motor_driver_index = 5,
                .analog_reader_index = 4,
                .min_angle_rad = -0.785f, // -45d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = true,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.785f,
                .max_stop_rad = 2.356f,
            },
            .hip_pitch = {
                .motor_driver_index = 6,
                .analog_reader_index = 5,
                .min_angle_rad = -2.356f, // -135d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = true,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.000f,
                .max_stop_rad = 3.142f,
            },
            .knee_pitch = {
                .motor_driver_index = 7,
                .analog_reader_index = 6,
                .min_angle_rad = 0.0f, // 0d
                .max_angle_rad = 2.618f, // 150d
                .is_inverted = false,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.262f,
                .max_stop_rad = 2.880f,
            },
            .contact_sensor_index = 7,
            .has_y_inverted = true,
        },
        .leg_br = {
            .hip_roll = {
                .motor_driver_index = 8,
                .analog_reader_index = 8,
                .min_angle_rad = -0.785f, // -45d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = false,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.785f,
                .max_stop_rad = 2.356f,
            },
            .hip_pitch = {
                .motor_driver_index = 9,
                .analog_reader_index = 9,
                .min_angle_rad = -2.356f, // -135d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = false,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.000f,
                .max_stop_rad = 3.142f,
            },
            .knee_pitch = {
                .motor_driver_index = 10,
                .analog_reader_index = 10,
                .min_angle_rad = 0.0f, // 0d
                .max_angle_rad = 2.618f, // 150d
                .is_inverted = true,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.262f,
                .max_stop_rad = 2.880f,
            },
            .contact_sensor_index = 11,
            .has_y_inverted = false,
        },
        .leg_fr = {
            .hip_roll = {
                .motor_driver_index = 11,
                .analog_reader_index = 12,
                .min_angle_rad = -0.785f, // -45d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = true,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.785f,
                .max_stop_rad = 2.356f,
            },
            .hip_pitch = {
                .motor_driver_index = 12,
                .analog_reader_index = 13,
                .min_angle_rad = -2.356f, // -135d
                .max_angle_rad = 0.785f, // 45d
                .is_inverted = false,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.000f,
                .max_stop_rad = 3.142f,
            },
            .knee_pitch = {
                .motor_driver_index = 13,
                .analog_reader_index = 14,
                .min_angle_rad = 0.0f, // 0d
                .max_angle_rad = 2.618f, // 150d
                .is_inverted = true,
                .has_feedback = true,
                .has_mechanical_stops = true,
                .min_stop_rad = 0.262f,
                .max_stop_rad = 2.880f,
            },
            .contact_sensor_index = 15,
            .has_y_inverted = false,
        }
    },
    .wifi = {
        .max_retries = 5,
        .ap_ssid = "TNY-360",
        .ap_password = "",
    },
};