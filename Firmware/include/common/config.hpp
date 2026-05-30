#pragma once
#include <cstdint>
#include <driver/gpio.h>

/** COMPILATION FLAGS **/
#define DEBUG_MODE 1  // 1 to enable debug logs and behaviors, 0 to disable

/** MULTICORE SETUP */
constexpr int CORE_BRAIN = 0;
constexpr int CORE_REFLEX = 1;
// number of milliseconds before triggering security stop (in case brain core crashes, to clear control intent)
constexpr uint32_t CONTROL_INTENT_WATCHDOG_MS = 500;

/** OTA SETUP **/
constexpr const char* OTA_FIRMWARE_LATEST_URL = "https://api.tny-robotics.com/firmware/latest";
constexpr const char* OTA_FILESYSTEM_DOWNLOAD_URL = "https://cdn.tny-robotics.com/firmware/tny-360/%s/filesystem.bin";
constexpr int OTA_UPDATE_TIMEOUT_MS = 5000;
constexpr int OTA_UPDATE_FILESYSTEM_BUFFER_SIZE = 4096; // in bytes
constexpr const char* OTA_ROBOT_MODEL = "tny-360";


/** PHYSICAL INFORMATIONS **/
constexpr float LEG_THIGH_LENGTH_M = 0.100f; // in meters
constexpr float LEG_CALF_LENGTH_M = 0.100f; // in meters
constexpr float HIP_OFFSET_M = 0.034f; // in meters
constexpr float HIP_POS_X_M = 0.086f; // in meters, from body center
constexpr float HIP_POS_Y_M = 0.052f; // in meters, from body center

/** DEFAULT POSE INFORMATIONS **/
constexpr float DEFAULT_BODY_HEIGHT_M   = 0.120f; // in meters, from ground level
constexpr float DEFAULT_FEET_SPREAD_X_M = 0.090f; // in meters, from body center
constexpr float DEFAULT_FEET_SPREAD_Y_M = 0.100f; // in meters, from body center


/** LOGGING **/
// Maximum number of log lines to store
constexpr int LOG_MAX_LINES = 20;
// Maximum length of each log message
constexpr int LOG_MAX_MSG_LEN = 256;

/** FILESYSTEM **/
// Maximum path length for file operations
constexpr int MAX_PATH_LEN = 128;

/** RPC **/
constexpr int RPC_QUEUE_SIZE = 32; // number of pending RPC jobs (between core 0 and core 1)


/** Wi-Fi **/
// Maximum number of connection retries before giving up
constexpr uint8_t WIFI_MAX_RETRIES = 5;
// Maximum length for SSID and Password
constexpr uint8_t WIFI_MAX_SSID_LEN = 32;
constexpr uint8_t WIFI_MAX_PASSWORD_LEN = 64;
// AP SSID and Password
constexpr const char* WIFI_AP_SSID = "TNY-360";
constexpr const char* WIFI_AP_PASSWORD = ""; // open by default


/** Websocket **/
// Maximum message size for WebSocket frames
constexpr uint16_t WEBSOCKET_MAX_MSG_SIZE = 256; // in bytes


/** Protocol **/
// Maximum number of pending protocol commands
constexpr uint8_t PROTOCOL_MAX_PENDING_COMMANDS = 32;
// Maximum number of commands in the protocol
constexpr uint8_t PROTOCOL_MAX_COMMANDS_HANDLERS = 255;


/** I2C **/
// Primary I2C GPIO pins (for critical modules, like sensors)
constexpr gpio_num_t I2C_PRIMARY_SDA_GPIO_NUM = GPIO_NUM_21;
constexpr gpio_num_t I2C_PRIMARY_SCL_GPIO_NUM = GPIO_NUM_47;
// Secondary I2C GPIO pins (for less critical modules, like screens)
constexpr gpio_num_t I2C_SECONDARY_SDA_GPIO_NUM = GPIO_NUM_9;
constexpr gpio_num_t I2C_SECONDARY_SCL_GPIO_NUM = GPIO_NUM_48;


/** Analog Readings **/
// EMA filter alpha value (1.0 = no filtering, 0.1 = big inertia)
constexpr float ANALOG_EMA_ALPHA = 0.05f;

/** Timer management **/
// Control loop frequency
constexpr int CONTROL_LOOP_FREQ_HZ = 200; // Hz
// Resolution of the control loop timer
constexpr int TIMER_RESOLUTION = 1'000'000; // 1Mhz, 1 us
// [][] calculated values below, do not edit manually
constexpr int TIMER_ALARM_COUNT = TIMER_RESOLUTION / CONTROL_LOOP_FREQ_HZ;
constexpr float CONTROL_LOOP_DT_S = 1.0f / CONTROL_LOOP_FREQ_HZ;
constexpr int CONTROL_LOOP_DT_MS = static_cast<int>(CONTROL_LOOP_DT_S * 1000);

// Decision loop frequency
constexpr int DECISION_LOOP_FREQ_HZ = 30; // Hz
constexpr float DECISION_LOOP_DT_S = 1.0f / DECISION_LOOP_FREQ_HZ;
constexpr int DECISION_LOOP_DT_MS = static_cast<int>(DECISION_LOOP_DT_S * 1000);

/** Analog scanner **/
// GPIO pins for the 4-bit multiplexer select lines
constexpr gpio_num_t SCANNER_SLCT_PIN1 = GPIO_NUM_39;
constexpr gpio_num_t SCANNER_SLCT_PIN2 = GPIO_NUM_40;
constexpr gpio_num_t SCANNER_SLCT_PIN3 = GPIO_NUM_41;
constexpr gpio_num_t SCANNER_SLCT_PIN4 = GPIO_NUM_42;


/** LEG **/
// Voltage threshold at which the leg is considered as grounded (touching ground)
constexpr float LEG_GROUNDED_THRESHOLD_V = 2.0f;


/** IMU **/
// I2C address for the IMU (MPU6050)
constexpr uint8_t IMU_I2C_ADDR = 0x68;
// I2C clock speed for IMU communication
constexpr uint32_t IMU_I2C_CLOCK = 100000; // 100kHz, could be up to 400kHz but i'm not 100% confident in the wiring
// number of samples to gather for calibration
constexpr uint16_t IMU_NB_CALIB_SAMPLES = 100;
// NOTE : Internal robot imu update is driven by the main timer at 200Hz


/** Motor **/
// I2C address for the motor driver (PCA9685)
constexpr uint8_t MOTOR_DRIVER_I2C_ADDR = 0x40;
// I2C clock speed for motor driver communication
constexpr uint32_t MOTOR_DRIVER_I2C_CLOCK = 400'000; // Hz
// PWM frequency for the motor driver (standard servo frequency)
constexpr uint16_t MOTOR_DRIVER_PWM_FREQUENCY_HZ = 200; // In Hz.
// NOTE : Internal robot motor update is driven by the main timer at CONTROL_LOOP_FREQ_HZ


/** Joint **/
constexpr uint8_t JOINT_COUNT = 16; // 12 legs joints + 2 ears, but PCA9685 has 16 channels

/** Screen **/
constexpr int SCREEN_REFRESH_RATE = 30;

/** Buttons **/
constexpr gpio_num_t BTN_LEFT_PIN = GPIO_NUM_11;
constexpr gpio_num_t BTN_RIGHT_PIN = GPIO_NUM_10;
constexpr uint16_t BTN_LONG_PRESS_MS = 300; // ms
// Buttons polling interval in milliseconds
constexpr uint16_t BTN_POLL_INT_MS = 50; // ms

/** Menu **/
// List item shift by default
constexpr uint8_t MENU_LIST_ITEM_DEFAULT_SHIFT = 4;
// List item shift when selected
constexpr uint8_t MENU_LIST_ITEM_SELECTED_SHIFT = 8;


/** Speaker **/
constexpr gpio_num_t SPEAKER_GPIO_NUM = GPIO_NUM_1;
constexpr int SPEAKER_SAMPLE_RATE_HZ = 22'500; // in Hz
constexpr size_t SPEAKER_NB_AUDIO_PROVIDERS = 4; // number of stacked audio providers
