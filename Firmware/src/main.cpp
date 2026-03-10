#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "Robot.hpp"
#include "common/config.hpp"
#include "audio/WavProvider.hpp"
#include "audio/SineProvider.hpp"
#include "drivers/CameraDriver.hpp"
#include "ui/LED.hpp"

#include "qrcodegen.h"

static const char* TAG = "Main";

static Robot robot;

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    Log::Add(Log::Level::Info, TAG, "\n\n====>> Starting robot (FIRMWARE_VERSION=%s) ...", FIRMWARE_VERSION);

    char wifi_str[100];
    if (strlen(WIFI_AP_PASSWORD) > 0) // Password (WPA/WPA2)
    {
        snprintf(wifi_str, sizeof(wifi_str), "WIFI:T:WPA;S:%s;P:%s;;", WIFI_AP_SSID, WIFI_AP_PASSWORD);
    }
    else // No password (Open)
    {
        snprintf(wifi_str, sizeof(wifi_str), "WIFI:T:nopass;S:%s;;", WIFI_AP_SSID);
    }

    uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(4)];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(4)];

    bool ok = qrcodegen_encodeText(wifi_str, tempBuffer, qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, 4, qrcodegen_Mask_AUTO, true);
    int size = qrcodegen_getSize(qrcode);

    // // wait a bit
    // vTaskDelay(pdMS_TO_TICKS(500));

    // // init camera
    // CameraDriver camera;
    // if (Error err = camera.init(); err != Error::None)
    // {
    //     Log::Add(Log::Level::Error, TAG, "Camera init failed with error: %s", ErrorToString(err));
    //     return;
    // }

    // // wait again
    // vTaskDelay(pdMS_TO_TICKS(200));

    // init the rest

    if (Error err = robot.init(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Robot Init failed with error: %s", ErrorToString(err));
        return;
    }

    Log::Add(Log::Level::Info, TAG, "Robot initialized successfully.");

    Log::Add(Log::Level::Info, TAG, "Enabling body motors.");
    if (Error err = robot.getBody().enable(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to enable body motors. Error: %s", ErrorToString(err));
        return;
    }

    // clamping joint velocity to stand up (safety measure)
    Joint::ClampVelocity(0.5f); // rad/s

    Transformf stand_position{
        Vec3f{20.f, 0.f, DEFAULT_BODY_HEIGHT_MM},
        Quatf{}
    };
    if (Error err = robot.getBody().setPosture(stand_position); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to set body target position. Error: %s", ErrorToString(err));
        return;
    }
    robot.getBody().getLeftEar().setTarget(DEG_TO_RAD(45.f));
    robot.getBody().getRightEar().setTarget(DEG_TO_RAD(45.f));

    // wait a bit for the robot to stand up
    vTaskDelay(pdMS_TO_TICKS(4000));

    // Reset joint velocity clamping and we're ready to go!
    Joint::ClampVelocity(Joint::MAX_VELOCITY_RAD_S); // could also set to 0 to disable clamping, either way works

    // AudioManager& audio_manager = robot.getAudioManager();

    // Log::Add(Log::Level::Info, TAG, "Initializing audio manager...");
    // if (Error err = audio_manager.init(); err != Error::None)
    // {
    //     Log::Add(Log::Level::Error, TAG, "Failed to initialize audio manager. Error: %s", ErrorToString(err));
    //     return;
    // }
    
    Log::Add(Log::Level::Info, TAG, ">>> Robot is operational.");
    // TODO : Play a litle sound to indicate we're ready (when sound system is working)

    // audio_manager.getMixer().setVolume(0.2f);

    // // Test audio with sinwave
    // robot.getAudioManager().getMixer().addSoundProvider(new SineProvider(440.0f, 1.f)); // A4 note at half volume

    // // Test audio with sound file
    // // WavProvider wav;
    // // if (Error err = wav.loadFromFile("What the dog doin.wav"); err != Error::None)
    // // {
    // //     Log::Add(Log::Level::Error, TAG, "Failed to load test wav file. Error: %s", ErrorToString(err));
    // //     return;
    // // }
    // // robot.getAudioManager().getMixer().addAudioProvider(new WavProvider(wav));
}
#ifdef __cplusplus
}
#endif