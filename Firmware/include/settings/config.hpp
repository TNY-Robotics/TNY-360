#pragma once
#include <ArduinoJson.h>
#include <iostream>

struct I2CBusConfig {
    int gpio_sda;
    int gpio_scl;
};

struct I2CConfig {
    I2CBusConfig primary;
    I2CBusConfig secondary;
};

struct SpeakerPDMConfig {
    int gpio_out;
};

struct SpeakerI2SConfig {
    int gpio_clk;
    int gpio_ws;
    int gpio_data;
};

enum class AudioSpeakerType {
    SPEAKER_PDM,
    SPEAKER_I2S,
    SPEAKER_NONE
};

struct AudioConfig {
    AudioSpeakerType speakerType;
    union {
        SpeakerPDMConfig pdm;
        SpeakerI2SConfig i2s;
    };
    float volume;
    int sampleRate;
    int nb_providers;
};

struct CameraGpioConfig {
    int pin_pwdn;
    int pin_reset;
    int pin_xclk;
    int pin_sccb_sda;
    int pin_sccb_scl;

    int pin_d7;
    int pin_d6;
    int pin_d5;
    int pin_d4;
    int pin_d3;
    int pin_d2;
    int pin_d1;
    int pin_d0;

    int pin_vsync;
    int pin_href;
    int pin_pclk;
};

struct CameraConfig {
    CameraGpioConfig gpio;
    int xclk_freq_hz;
};

enum class ScreenType {
    SCREEN_NONE,
    SCREEN_SH1106,
    SCREEN_SSD1309,
};

struct ScreenConfig {
    ScreenType type;
};

struct ButtonsConfig {
    int gpio_left;
    int gpio_right;
    int polling_interval_ms;
    int long_press_duration_ms;
};

struct AnalogConfig {
    int gpio_select_1;
    int gpio_select_2;
    int gpio_select_3;
    int gpio_select_4;
    int channel;
};

enum class IMUType {
    IMU_NONE,
    IMU_MPU6050,
    IMU_LSM6DS3
};

struct IMUConfig {
    IMUType type;
};

struct RobotConfig {
    I2CConfig i2c;
    AudioConfig audio;
    CameraConfig camera;
    ScreenConfig screen;
    ButtonsConfig buttons;
    AnalogConfig analog;
    IMUConfig imu;

    ////////////
    // Helper functions to load/save the config from/to JSON
    ////////////

    /**
     * @brief Load the configuration from a JSON input stream.
     * @param inputStream The input stream containing the JSON data.
     */
    void loadFromStream(std::istream &inputStream);

    /**
     * @brief Load the configuration from a JSON string.
     * @param jsonString The JSON string containing the configuration data.
     */
    void loadFromString(const char* jsonString);

    /**
     * @brief Save the configuration to a buffer as a JSON string.
     * @param outputBuffer The buffer to store the JSON string.
     * @param maxLen The maximum length of the output buffer.
     * @return The number of bytes written to the buffer.
     */
    size_t saveToBuffer(char* outputBuffer, size_t maxLen);

    /**
     * @brief Save the configuration to a JSON output stream.
     * @param outputStream The output stream to write the JSON data.
     * @return True if the operation was successful, false otherwise.
     */
    bool saveToStream(std::ostream &outputStream);
};

extern RobotConfig config;
