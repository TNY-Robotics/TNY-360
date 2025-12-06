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
#include "modules/Timer.hpp"
#include "modules/Button.hpp"
#include "modules/Screen.hpp"
#include "modules/Menus.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    Log::Add(Log::Level::Info, "Starting robot ...");
    
    // Initialize LED Module
    Log::Add(Log::Level::Info, "Initializing LED Module...");
    if (Error err = LED::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "LED Module Init failed with error: %d", static_cast<uint8_t>(err));
    }
    else
    {
        Log::Add(Log::Level::Info, "LED Module initialized");
    }

    LED::SetColor(0, {64, 32, 0}, 0.2f); // Set LED 0 to low orange as startup indicator

    // Initialize Button Module
    Log::Add(Log::Level::Info, "Initializing Button Module...");
    if (Error err = Button::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Button Module Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x15); // 0b00010101
    }
    else
    {
        Log::Add(Log::Level::Info, "Button Module initialized");
    }

    // Initialize NVS
    Log::Add(Log::Level::Info, "Initializing NVS...");
    if (Error err = NVS::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "NVS Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0xaa); // 0b10101010
    }
    else
    {
        Log::Add(Log::Level::Info, "NVS initialized");
    }

    // Initialize Analog Scanner
    Log::Add(Log::Level::Info, "Initializing Analog Scanner...");
    if (Error err = AnalogScanner::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Analog Scanner Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x33); // 0b001100110011
    }
    else
    {
        Log::Add(Log::Level::Info, "Analog Scanner initialized");
    }

    // Initialize Pressure Sensor
    Log::Add(Log::Level::Info, "Initializing Pressure Sensor...");
    if (Error err = Pressure::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Pressure Sensor Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x0f); // 0b00001111
    }
    else
    {
        Log::Add(Log::Level::Info, "Pressure Sensor initialized");
    }

    // Initialize Sound Module
    Log::Add(Log::Level::Info, "Initializing Sound Module...");
    if (Error err = Sound::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Sound Module Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0xf0); // 0b11110000
    }
    else
    {
        Log::Add(Log::Level::Info, "Sound Module initialized");
    }

    // Initialize I2C
    Log::Add(Log::Level::Info, "Initializing I2C...");
    if (Error err = I2C::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "I2C Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0xe7); // 0b11100111
    }
    else
    {
        Log::Add(Log::Level::Info, "I2C initialized");
    }

    // FIXME : temporary test to list I2C Devices
    {
        Log::Add(Log::Level::Info, "Scanning primary I2C bus for devices...");
        for (uint8_t addr = 1; addr < 127; addr++)
        {
            if (I2C::ProbeAddress(I2C::handle_primary, addr) == Error::Ok)
            {
                Log::Add(Log::Level::Info, "Found I2C device at address 0x%02x", addr);
            }
        }

        Log::Add(Log::Level::Info, "Scanning secondary I2C bus for devices...");
        for (uint8_t addr = 1; addr < 127; addr++)
        {
            if (I2C::ProbeAddress(I2C::handle_secondary, addr) == Error::Ok)
            {
                Log::Add(Log::Level::Info, "Found I2C device at address 0x%02x", addr);
            }
        }
    }

    // Initialize Motor Control
    Log::Add(Log::Level::Info, "Initializing Motor Control...");
    bool MotorInitialized = false;
    if (Error err = Motor::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Motor Control Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x57); // 0b01010111
    }
    else
    {
        Log::Add(Log::Level::Info, "Motor Control initialized");
        MotorInitialized = true;
    }

    // Initialize IMU
    Log::Add(Log::Level::Info, "Initializing IMU...");
    bool IMUInitialized = false;
    if (Error err = IMU::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "IMU Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0xac); // 0b10101100
    }
    else
    {
        Log::Add(Log::Level::Info, "IMU initialized");
        IMUInitialized = true;
    }

    // Initialize Screen Module
    Log::Add(Log::Level::Info, "Initializing Screen Module...");
    if (Error err = Screen::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Screen Module Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x3c); // 0b00111100
    }
    else
    {
        Log::Add(Log::Level::Info, "Screen Module initialized");
    }

    // Initialize Menus system
    Log::Add(Log::Level::Info, "Initializing Menus System...");
    if (Error err = Menus::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Menus System Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x2a); // 0b00101010
    }
    else
    {
        Log::Add(Log::Level::Info, "Menus System initialized");
    }

    // Initialize Power Management
    Log::Add(Log::Level::Info, "Initializing Power Management...");
    if (Error err = Power::Init(); err != Error::Ok)
    {
        Log::Add(Log::Level::Error, "Power Management Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x39); // 0b00111001
    }
    else
    {
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
                    },
                    [](Error err) { // on failure, log error
                        Log::Add(Log::Level::Error, "Failed to start AP, this might be a serious issue.");
                    }
                );
            }
        );
    }
    else
    {
        Log::Add(Log::Level::Error, "Wi-Fi Init failed with error: %d", static_cast<uint8_t>(err));
        LED::LoopErrorCode(0x4e); // 0b01001110
    }

    // Initialize Main Timer (starts main lifecycle tasks like IMU updates, motor control, etc.)
    if (MotorInitialized && IMUInitialized)
    {
        Log::Add(Log::Level::Info, "Initializing Main Timer...");
        if (Error err = Timer::Init(); err != Error::Ok)
        {
            Log::Add(Log::Level::Error, "Main Timer Init failed with error: %d", static_cast<uint8_t>(err));
            LED::LoopErrorCode(0x69); // 0b01101001
        }
        else
        {
            Log::Add(Log::Level::Info, "Main Timer initialized");
        }
    } else
    {
        Log::Add(Log::Level::Warning, "Main Timer initialization skipped due to previous errors.");
    }

    LED::SetColor(0, {0, 64, 0}, 0.2f); // Set LED 0 to low green to indicate ready state

    Log::Add(Log::Level::Info, "Robot initialization complete.");

    Sound::SetVolume(0.2f);
    Sound::Play(Sound::note(Sound::Freq::C_5, 0.05f));
    Sound::Play(Sound::pause(0.04f));
    Sound::Play(Sound::note(Sound::Freq::C_5, 0.05f));
    Sound::Play(Sound::pause(0.02f));
    Sound::Play(Sound::note(Sound::Freq::A_5, 0.05f));

    // Other sounds below for testing purposes
    
    // Sound::Play(Sound::note(Sound::Freq::E_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::F_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::A_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::B_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::A_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::B_4, 0.05f));
    
    // Sound::Play(Sound::note(Sound::Freq::A_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::E_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::F_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::A_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::B_4, 0.05f));
    // Sound::Play(Sound::pause(0.02f));
    // Sound::Play(Sound::note(Sound::Freq::C_4, 0.05f));
}
#ifdef __cplusplus
}
#endif