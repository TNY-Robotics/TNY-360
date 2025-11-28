#pragma once
#include <cstdint>
#include <driver/gpio.h>

/** COMPILATION FLAGS **/
#define DEBUG_MODE 1  // 1 to enable debug logs and behaviors, 0 to disable


/** LOGGING **/
// Maximum number of log lines to store
constexpr uint8_t LOG_MAX_LINES = 10;
// Maximum length of each log message
constexpr uint8_t LOG_MAX_MSG_LEN = 128;

/** FILESYSTEM **/
// Maximum path length for file operations
constexpr uint8_t MAX_PATH_LEN = 128;


/** Wi-Fi **/
// Maximum number of connection retries before giving up
constexpr uint8_t WIFI_MAX_RETRIES = 5;
// Maximum length for SSID and Password
constexpr uint8_t WIFI_MAX_SSID_LEN = 32;
constexpr uint8_t WIFI_MAX_PASSWORD_LEN = 64;
// AP SSID and Password
constexpr const char* WIFI_AP_SSID = "TNY-360";
constexpr const char* WIFI_AP_PASSWORD = "tny-360-pass";


/** Websocket **/
// Maximum message size for WebSocket frames
constexpr uint16_t WEBSOCKET_MAX_MSG_SIZE = 256; // in bytes


/** Protocol **/
// Maximum number of pending protocol calls
constexpr uint8_t PROTOCOL_MAX_PENDING_CALLS = 16;


/** I2C **/
// Primary I2C GPIO pins (for critical modules, like sensors)
constexpr gpio_num_t I2C_PRIMARY_SDA_GPIO_NUM = GPIO_NUM_47;
constexpr gpio_num_t I2C_PRIMARY_SCL_GPIO_NUM = GPIO_NUM_21;
// Secondary I2C GPIO pins (for less critical modules, like screens)
constexpr gpio_num_t I2C_SECONDARY_SDA_GPIO_NUM = GPIO_NUM_38;
constexpr gpio_num_t I2C_SECONDARY_SCL_GPIO_NUM = GPIO_NUM_45;


/** Buttons **/
constexpr gpio_num_t BTN_LEFT_PIN = GPIO_NUM_32;
constexpr gpio_num_t BTN_RIGHT_PIN = GPIO_NUM_33;
constexpr uint16_t BTN_LONG_PRESS_MS = 500; // ms
// Buttons polling interval in milliseconds
constexpr uint16_t BTN_POLL_INT_MS = 50; // ms

/** Timer management **/
// Main timer frequency in Hz
constexpr uint16_t MAIN_TIMER_FREQUENCY_HZ = 800;
// NOTE : 800Hz is chosen because we need to sample 16 channels at 50Hz (for motors + sensors),
//        so we need at least 16*50 = 800 updates per second.
constexpr uint16_t SECONDARY_FREQUENCY_HZ = 50; // 50Hz for slower updates (imu, motors, etc)
constexpr uint16_t TIMER_DIVIDER_SECONDARY = MAIN_TIMER_FREQUENCY_HZ / SECONDARY_FREQUENCY_HZ;

/** Analog scanner **/
// GPIO pins for the 4-bit multiplexer select lines
constexpr gpio_num_t SCANNER_SLCT_PIN1 = GPIO_NUM_39;
constexpr gpio_num_t SCANNER_SLCT_PIN2 = GPIO_NUM_40;
constexpr gpio_num_t SCANNER_SLCT_PIN3 = GPIO_NUM_41;
constexpr gpio_num_t SCANNER_SLCT_PIN4 = GPIO_NUM_42;


/** IMU **/
// I2C address for the IMU (MPU6050)
constexpr uint8_t IMU_I2C_ADDR = 0x68;
// I2C clock speed for IMU communication
constexpr uint32_t IMU_I2C_CLOCK = 100000; // 100kHz, could be up to 400kHz but i'm not 100% confident in the wiring
// number of samples to gather for calibration
constexpr uint16_t IMU_NB_CALIB_SAMPLES = 100;
// NOTE : Internal robot imu update is driven by the main timer at 50Hz

/** Motor **/
// I2C address for the motor driver (PCA9685)
constexpr uint8_t MOTOR_DRIVER_I2C_ADDR = 0x40;
// I2C clock speed for motor driver communication
constexpr uint32_t MOTOR_DRIVER_I2C_CLOCK = 100000; // 100kHz, could be up to 400kHz but i'm not 100% confident in the wiring
// PWM frequency for the motor driver (standard servo frequency)
constexpr uint16_t MOTOR_DRIVER_PWM_FREQUENCY_HZ = 50; // 50Hz
// NOTE : Internal robot motor update is driven by the main timer at 50Hz