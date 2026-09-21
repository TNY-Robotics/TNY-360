#include "settings/Settings.hpp"
#include "common/Log.hpp"

constexpr static const char* TAG = "Settings";


////////////
// Serialization and deserialization functions (for ArduinoJson)
////////////


/// I2CBusConfig
inline void convertToJson(const I2CBusConfig& src, JsonVariant& dst)
{
    dst["gpio_sda"] = src.gpio_sda;
    dst["gpio_scl"] = src.gpio_scl;
}
inline void convertFromJson(JsonVariantConst src, I2CBusConfig& dst) {
    dst.gpio_sda = src["gpio_sda"] | dst.gpio_sda;
    dst.gpio_scl = src["gpio_scl"] | dst.gpio_scl;
}
inline bool canConvertFromJson(JsonVariantConst src, I2CBusConfig& dst) { return src.is<JsonObject>(); }

/// I2CConfig
inline void convertToJson(const I2CConfig& src, JsonVariant& dst)
{
    dst["primary"] = src.primary;
    dst["secondary"] = src.secondary;
}
inline void convertFromJson(JsonVariantConst src, I2CConfig& dst) {
    dst.primary = src["primary"] | dst.primary;
    dst.secondary = src["secondary"] | dst.secondary;
}
inline bool canConvertFromJson(JsonVariantConst src, I2CConfig& dst) { return src.is<JsonObject>(); }

/// SpeakerPDMConfig
inline void convertToJson(const SpeakerPDMConfig& src, JsonVariant& dst)
{
    dst["gpio_out"] = src.gpio_out;
}
inline void convertFromJson(JsonVariantConst src, SpeakerPDMConfig& dst) {
    dst.gpio_out = src["gpio_out"] | dst.gpio_out;
}
inline bool canConvertFromJson(JsonVariantConst src, SpeakerPDMConfig& dst) { return src.is<JsonObject>(); }

/// SpeakerI2SConfig
inline void convertToJson(const SpeakerI2SConfig& src, JsonVariant& dst)
{
    dst["gpio_clk"] = src.gpio_clk;
    dst["gpio_ws"] = src.gpio_ws;
    dst["gpio_data"] = src.gpio_data;
}
inline void convertFromJson(JsonVariantConst src, SpeakerI2SConfig& dst) {
    dst.gpio_clk = src["gpio_clk"] | dst.gpio_clk;
    dst.gpio_ws = src["gpio_ws"] | dst.gpio_ws;
    dst.gpio_data = src["gpio_data"] | dst.gpio_data;
}
inline bool canConvertFromJson(JsonVariantConst src, SpeakerI2SConfig& dst) { return src.is<JsonObject>(); }

/// AudioConfig
inline void convertToJson(const AudioConfig& src, JsonVariant& dst)
{
    if (src.speakerType == AudioSpeakerType::SPEAKER_PDM) {
        dst["speakerType"] = "PDM";
        dst["pdm"] = src.pdm;
    } else if (src.speakerType == AudioSpeakerType::SPEAKER_I2S) {
        dst["speakerType"] = "I2S";
        dst["i2s"] = src.i2s;
    } else {
        dst["speakerType"] = "NONE";
    }
    dst["volume"] = src.volume;
    dst["sampleRate"] = src.sampleRate;
    dst["nb_providers"] = src.nb_providers;
}
inline void convertFromJson(JsonVariantConst src, AudioConfig& dst)
{
    std::string speakerTypeStr = src["speakerType"] | "NONE";
    if (speakerTypeStr == "PDM") {
        dst.speakerType = AudioSpeakerType::SPEAKER_PDM;
        dst.pdm = src["pdm"] | dst.pdm;
    } else if (speakerTypeStr == "I2S") {
        dst.speakerType = AudioSpeakerType::SPEAKER_I2S;
        dst.i2s = src["i2s"] | dst.i2s;
    } else {
        dst.speakerType = AudioSpeakerType::SPEAKER_NONE;
    }
    dst.volume = src["volume"] | dst.volume;
    dst.sampleRate = src["sampleRate"] | dst.sampleRate;
    dst.nb_providers = src["nb_providers"] | dst.nb_providers;
}
inline bool canConvertFromJson(JsonVariantConst src, AudioConfig& dst) { return src.is<JsonObject>(); }

/// CameraGpioConfig
inline void convertToJson(const CameraGpioConfig& src, JsonVariant& dst)
{
    dst["pin_pwdn"] = src.pin_pwdn;
    dst["pin_reset"] = src.pin_reset;
    dst["pin_xclk"] = src.pin_xclk;
    dst["pin_sccb_sda"] = src.pin_sccb_sda;
    dst["pin_sccb_scl"] = src.pin_sccb_scl;

    dst["pin_d7"] = src.pin_d7;
    dst["pin_d6"] = src.pin_d6;
    dst["pin_d5"] = src.pin_d5;
    dst["pin_d4"] = src.pin_d4;
    dst["pin_d3"] = src.pin_d3;
    dst["pin_d2"] = src.pin_d2;
    dst["pin_d1"] = src.pin_d1;
    dst["pin_d0"] = src.pin_d0;

    dst["pin_vsync"] = src.pin_vsync;
    dst["pin_href"] = src.pin_href;
    dst["pin_pclk"] = src.pin_pclk;
}
inline void convertFromJson(JsonVariantConst src, CameraGpioConfig& dst)
{
    dst.pin_pwdn = src["pin_pwdn"] | dst.pin_pwdn;
    dst.pin_reset = src["pin_reset"] | dst.pin_reset;
    dst.pin_xclk = src["pin_xclk"] | dst.pin_xclk;
    dst.pin_sccb_sda = src["pin_sccb_sda"] | dst.pin_sccb_sda;
    dst.pin_sccb_scl = src["pin_sccb_scl"] | dst.pin_sccb_scl;

    dst.pin_d7 = src["pin_d7"] | dst.pin_d7;
    dst.pin_d6 = src["pin_d6"] | dst.pin_d6;
    dst.pin_d5 = src["pin_d5"] | dst.pin_d5;
    dst.pin_d4 = src["pin_d4"] | dst.pin_d4;
    dst.pin_d3 = src["pin_d3"] | dst.pin_d3;
    dst.pin_d2 = src["pin_d2"] | dst.pin_d2;
    dst.pin_d1 = src["pin_d1"] | dst.pin_d1;
    dst.pin_d0 = src["pin_d0"] | dst.pin_d0;

    dst.pin_vsync = src["pin_vsync"] | dst.pin_vsync;
    dst.pin_href = src["pin_href"] | dst.pin_href;
    dst.pin_pclk = src["pin_pclk"] | dst.pin_pclk;
}
inline bool canConvertFromJson(JsonVariantConst src, CameraGpioConfig& dst) { return src.is<JsonObject>(); }

// CameraConfig
inline void convertToJson(const CameraConfig& src, JsonVariant& dst)
{
    dst["gpio"] = src.gpio;
    dst["xclk_freq_hz"] = src.xclk_freq_hz;
}
inline void convertFromJson(JsonVariantConst src, CameraConfig& dst)
{
    dst.gpio = src["gpio"] | dst.gpio;
    dst.xclk_freq_hz = src["xclk_freq_hz"] | dst.xclk_freq_hz;
}
inline bool canConvertFromJson(JsonVariantConst src, CameraConfig& dst) { return src.is<JsonObject>(); }

/// ScreenConfig
inline void convertToJson(const ScreenConfig& src, JsonVariant& dst)
{
    if (src.type == ScreenType::SCREEN_SH1106) { dst["type"] = "SH1106"; }
    else if (src.type == ScreenType::SCREEN_SSD1309) { dst["type"] = "SSD1309"; }
    else { dst["type"] = "NONE"; }
}
inline void convertFromJson(JsonVariantConst src, ScreenConfig& dst)
{
    std::string screenTypeStr = src["type"] | "NONE";
    if (screenTypeStr == "SH1106") { dst.type = ScreenType::SCREEN_SH1106; }
    else if (screenTypeStr == "SSD1309") { dst.type = ScreenType::SCREEN_SSD1309; }
    else { dst.type = ScreenType::SCREEN_NONE; }
}
inline bool canConvertFromJson(JsonVariantConst src, ScreenConfig& dst) { return src.is<JsonObject>(); }

/// ButtonsConfig
inline void convertToJson(const ButtonsConfig& src, JsonVariant& dst)
{
    dst["gpio_left"] = src.gpio_left;
    dst["gpio_right"] = src.gpio_right;
    dst["polling_interval_ms"] = src.polling_interval_ms;
    dst["long_press_duration_ms"] = src.long_press_duration_ms;
}
inline void convertFromJson(JsonVariantConst src, ButtonsConfig& dst) {
    dst.gpio_left = src["gpio_left"] | dst.gpio_left;
    dst.gpio_right = src["gpio_right"] | dst.gpio_right;
    dst.polling_interval_ms = src["polling_interval_ms"] | dst.polling_interval_ms;
    dst.long_press_duration_ms = src["long_press_duration_ms"] | dst.long_press_duration_ms;
}
inline bool canConvertFromJson(JsonVariantConst src, ButtonsConfig& dst) { return src.is<JsonObject>(); }

/// AnalogConfig
inline void convertToJson(const AnalogConfig& src, JsonVariant& dst)
{
    dst["gpio_select_1"] = src.gpio_select_1;
    dst["gpio_select_2"] = src.gpio_select_2;
    dst["gpio_select_3"] = src.gpio_select_3;
    dst["gpio_select_4"] = src.gpio_select_4;
    dst["channel"] = src.channel;
}
inline void convertFromJson(JsonVariantConst src, AnalogConfig& dst) {
    dst.gpio_select_1 = src["gpio_select_1"] | dst.gpio_select_1;
    dst.gpio_select_2 = src["gpio_select_2"] | dst.gpio_select_2;
    dst.gpio_select_3 = src["gpio_select_3"] | dst.gpio_select_3;
    dst.gpio_select_4 = src["gpio_select_4"] | dst.gpio_select_4;
    dst.channel = src["channel"] | dst.channel;
}
inline bool canConvertFromJson(JsonVariantConst src, AnalogConfig& dst) { return src.is<JsonObject>(); }

/// IMUConfig
inline void convertToJson(const IMUConfig& src, JsonVariant& dst)
{
    if (src.type == IMUType::IMU_MPU6050) { dst["type"] = "MPU6050"; }
    else if (src.type == IMUType::IMU_LSM6DS3) { dst["type"] = "LSM6DS3"; }
    else { dst["type"] = "NONE"; }
}
inline void convertFromJson(JsonVariantConst src, IMUConfig& dst)
{
    std::string imuTypeStr = src["type"] | "NONE";
    if (imuTypeStr == "MPU6050") { dst.type = IMUType::IMU_MPU6050; }
    else if (imuTypeStr == "LSM6DS3") { dst.type = IMUType::IMU_LSM6DS3; }
    else { dst.type = IMUType::IMU_NONE; }
}
inline bool canConvertFromJson(JsonVariantConst src, IMUConfig& dst) { return src.is<JsonObject>(); }

/// RobotConfig
inline void convertToJson(const RobotConfig& src, JsonVariant& dst)
{
    dst["i2c"] = src.i2c;
    dst["audio"] = src.audio;
    dst["camera"] = src.camera;
    dst["screen"] = src.screen;
    dst["buttons"] = src.buttons;
    dst["analog"] = src.analog;
    dst["imu"] = src.imu;
}
inline void convertFromJson(JsonVariantConst src, RobotConfig& dst) {
    dst.i2c = src["i2c"] | dst.i2c;
    dst.audio = src["audio"] | dst.audio;
    dst.camera = src["camera"] | dst.camera;
    dst.screen = src["screen"] | dst.screen;
    dst.buttons = src["buttons"] | dst.buttons;
    dst.analog = src["analog"] | dst.analog;
    dst.imu = src["imu"] | dst.imu;
}
inline bool canConvertFromJson(JsonVariantConst src, RobotConfig& dst) { return src.is<JsonObject>(); }


////////////
// Helper functions
////////////


void RobotConfig::loadFromStream(std::istream &inputStream)
{
    JsonDocument doc;
    if (deserializeJson(doc, inputStream) == DeserializationError::Ok)
    {
        convertFromJson(doc.as<JsonVariantConst>(), *this);
    }
    else
    {
        LOG_ERROR(TAG, "Failed to deserialize JSON from stream");
    }
}

void RobotConfig::loadFromString(const char* jsonString)
{
    JsonDocument doc;
    if (deserializeJson(doc, jsonString) == DeserializationError::Ok)
    {
        convertFromJson(doc.as<JsonVariantConst>(), *this);
    }
    else
    {
        LOG_ERROR(TAG, "Failed to deserialize JSON from string");
    }
}

size_t RobotConfig::saveToBuffer(char* outputBuffer, size_t maxLen)
{
    JsonDocument doc;
    doc.set(*this);

    return serializeJsonPretty(doc, outputBuffer, maxLen);
}

bool RobotConfig::saveToStream(std::ostream &outputStream)
{
    JsonDocument doc;
    doc.set(*this);

    size_t bytesWritten = serializeJsonPretty(doc, outputStream);
    return bytesWritten > 0;
}