#include <freertos/FreeRTOS.h>
#include "modules/Log.hpp"
#include "modules/NVS.hpp"
#include "modules/WiFi.hpp"
#include "modules/Network.hpp"
#include "modules/Lang.hpp"
#include "modules/I2C.hpp"
#include "modules/IMU.hpp"
#include "modules/AnalogScanner.hpp"
#include "modules/Pressure.hpp"
#include "modules/Motor.hpp"
#include "modules/Sound.hpp"
#include "modules/Power.hpp"
#include "modules/LED.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    Log::Add(Log::Level::Info, "Starting robot ...");
    
    // Initialize LED Module
    Log::Add(Log::Level::Info, "Initializing LED Module...");
    if (Error err = LED::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "LED Module Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "LED Module initialized");
    }

    LED::SetColor(0, {64, 32, 0}, 0.2f); // Set LED 0 to orange as startup indicator

    // Initialize NVS
    Log::Add(Log::Level::Info, "Initializing NVS...");
    if (Error err = NVS::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "NVS Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "NVS initialized");
    }

    // TODO : Use 800Hz timer for analog scanner sampling + motor and imu management

    // Initialize Analog Scanner
    Log::Add(Log::Level::Info, "Initializing Analog Scanner...");
    if (Error err = AnalogScanner::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "Analog Scanner Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "Analog Scanner initialized");
    }

    // Initialize Pressure Sensor
    Log::Add(Log::Level::Info, "Initializing Pressure Sensor...");
    if (Error err = Pressure::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "Pressure Sensor Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "Pressure Sensor initialized");
    }

    // Initialize Sound Module
    Log::Add(Log::Level::Info, "Initializing Sound Module...");
    if (Error err = Sound::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "Sound Module Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "Sound Module initialized");
    }

    // Initialize I2C
    Log::Add(Log::Level::Info, "Initializing I2C...");
    if (Error err = I2C::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "I2C Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "I2C initialized");
    }

    // Initialize IMU
    Log::Add(Log::Level::Info, "Initializing IMU...");
    if (Error err = IMU::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "IMU Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "IMU initialized");
    }

    // Initialize Motor Control
    Log::Add(Log::Level::Info, "Initializing Motor Control...");
    if (Error err = Motor::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "Motor Control Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "Motor Control initialized");
    }

    // Initialize Power Management
    Log::Add(Log::Level::Info, "Initializing Power Management...");
    if (Error err = Power::Init(); err != Error::Ok) {
        Log::Add(Log::Level::Error, "Power Management Init failed with error: %d", static_cast<uint8_t>(err));
    } else {
        Log::Add(Log::Level::Info, "Power Management initialized");
    }
    
    // Initialize WiFi module
    Log::Add(Log::Level::Info, "Initializing WiFi...");
    if (Error err = WiFi::Init(); err == Error::Ok)
    {
        Log::Add(Log::Level::Info, "WiFi initialized, connecting to AP...");
        // Connect to AP, if error, start AP mode
        WiFi::Connect(
            []() { // on success, start web server and websocket
                Log::Add(Log::Level::Info, "Connected to WiFi, starting webserver...");
                if (Network::SetupSTA() != Error::Ok) {
                    Log::Add(Log::Level::Error, "Network SetupSTA failed");
                    return;
                }
                Log::Add(Log::Level::Info, "WebServer and websocket started");
                LED::SetColor(0, {0, 16, 0}, 0.2f); // Set LED 0 to green as a test
            },
            [](Error err) { // on failure, start AP
                Log::Add(Log::Level::Info, "Failed to connect to Wi-Fi, starting AP...");
                WiFi::StartAP(WIFI_AP_SSID, WIFI_AP_PASSWORD,
                    []() { // on success, start captive portal
                        Log::Add(Log::Level::Info, "AP started");
                        if (Network::SetupAP() != Error::Ok) {
                            Log::Add(Log::Level::Error, "Network SetupAP failed");
                            return;
                        }
                        Log::Add(Log::Level::Info, "Captive portal ready");
                        LED::SetColor(0, {0, 0, 16}, 0.2f); // Set LED 0 to green as a test
                    },
                    [](Error err) { // on failure, log error
                        Log::Add(Log::Level::Error, "Failed to start AP, this might be a serious issue.");
                    }
                );
            }
        );
    }
    else Log::Add(Log::Level::Error, "Wi-Fi Init failed with error: %d", static_cast<uint8_t>(err));

    Log::Add(Log::Level::Info, "Robot initialization complete.");
}
#ifdef __cplusplus
}
#endif