#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "Robot.hpp"
#include "common/config.hpp"
#include "audio/WavProvider.hpp"
#include "audio/SineProvider.hpp"

static const char* TAG = "Main";

static Robot robot;

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    Log::Add(Log::Level::Info, TAG, "\n\n====>> Starting robot (FIRMWARE_VERSION=%s) ...", FIRMWARE_VERSION);

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

    // wait a bit for the robot to stand up
    vTaskDelay(pdMS_TO_TICKS(4000));

    // Reset joint velocity clamping and we're ready to go!
    Joint::ClampVelocity(Joint::MAX_VELOCITY_RAD_S); // could also set to 0 to disable clamping, either way works
    
    Log::Add(Log::Level::Info, TAG, ">>> Robot is operational.");
    // TODO : Play a litle sound to indicate we're ready (when sound system is working)

    // Test audio with sinwave
    // robot.getAudioManager().getMixer().addSoundProvider(new SineProvider());

    // Test audio with sound file
    // WavProvider wav;
    // if (Error err = wav.loadFromFile("What the dog doin.wav"); err != Error::None)
    // {
    //     Log::Add(Log::Level::Error, TAG, "Failed to load test wav file. Error: %s", ErrorToString(err));
    //     return;
    // }
    // robot.getAudioManager().getMixer().addAudioProvider(new WavProvider(wav));
}
#ifdef __cplusplus
}
#endif