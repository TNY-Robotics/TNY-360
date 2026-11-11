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

/** DEFAULT POSE INFORMATIONS **/
constexpr float DEFAULT_BODY_HEIGHT_M   = 0.120f; // in meters, from ground level
constexpr float DEFAULT_FEET_SPREAD_X_M = 0.090f; // in meters, from body center
constexpr float DEFAULT_FEET_SPREAD_Y_M = 0.100f; // in meters, from body center


/** ERROR EVENTS **/
const int ERROR_MAX_EVENTS = 32;


/** LOGGING **/
// Maximum number of log lines to store
constexpr int LOG_MAX_LINES = 32;
// Maximum length of each log message
constexpr int LOG_MAX_MSG_LEN = 160;
// Maximum length of log tag
constexpr int LOG_MAX_TAG_LEN = 16;

/** FILESYSTEM **/
// Maximum path length for file operations
constexpr int MAX_PATH_LEN = 128;

/** RPC **/
constexpr int RPC_QUEUE_SIZE = 32; // number of pending RPC jobs (between core 0 and core 1)


/** Wi-Fi **/
// Maximum length for SSID and Password
constexpr uint8_t WIFI_MAX_SSID_LEN = 32;
constexpr uint8_t WIFI_MAX_PASSWORD_LEN = 64;


/** Websocket **/
// Maximum message size for WebSocket frames
constexpr uint16_t WEBSOCKET_MAX_MSG_SIZE = 256; // in bytes


/** Protocol **/
// Maximum number of pending protocol commands
constexpr uint8_t PROTOCOL_MAX_PENDING_COMMANDS = 32;
// Maximum number of commands in the protocol
constexpr uint8_t PROTOCOL_MAX_COMMANDS_HANDLERS = 255;

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


/** LEG **/
// Voltage threshold at which the leg is considered as grounded (touching ground)
constexpr float LEG_GROUNDED_THRESHOLD_V = 2.0f;


/** Motor **/
// I2C address for the motor driver (PCA9685)
constexpr uint8_t MOTOR_DRIVER_I2C_ADDR = 0x40;
// I2C clock speed for motor driver communication
constexpr uint32_t MOTOR_DRIVER_I2C_CLOCK = 400'000; // 1'000'000; // Hz
// PWM frequency for the motor driver (standard servo frequency)
constexpr float MOTOR_DRIVER_PWM_FREQUENCY_HZ = 200.f; // In Hz.
// NOTE : Internal robot motor update is driven by the main timer at CONTROL_LOOP_FREQ_HZ

/** Menu **/
// List item shift by default
constexpr uint8_t MENU_LIST_ITEM_DEFAULT_SHIFT = 4;
// List item shift when selected
constexpr uint8_t MENU_LIST_ITEM_SELECTED_SHIFT = 8;


/** Speaker **/
constexpr size_t SPEAKER_NB_AUDIO_PROVIDERS = 4; // number of stacked audio providers
