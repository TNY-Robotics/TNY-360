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
        Log::Add(Log::Level::Error, TAG, "Robot Init failed with error: %d", static_cast<uint8_t>(err));
        return;
    }

    Log::Add(Log::Level::Info, TAG, "Robot initialized successfully.");

    // Log::Add(Log::Level::Info, TAG, "Enabling body motors.");
    // if (Error err = robot.getBody().enable(); err != Error::None)
    // {
    //     Log::Add(Log::Level::Error, TAG, "Failed to enable body motors. Error: %d", static_cast<uint8_t>(err));
    //     return;
    // }

    // Transformf stand_position{
    //     Vec3f{0.f, 0.f, DEFAULT_BODY_HEIGHT_MM},
    //     Quatf{}
    // };
    // if (Error err = robot.getBody().setPosture(stand_position); err != Error::None)
    // {
    //     Log::Add(Log::Level::Error, TAG, "Failed to set body target position. Error: %d", static_cast<uint8_t>(err));
    //     return;
    // }

    Log::Add(Log::Level::Info, TAG, "Robot is operational.");

    // Test audio with sinwave
    // robot.getAudioManager().getMixer().addSoundProvider(new SineProvider());

    // Test audio with sound file
    // WavProvider wav;
    // if (Error err = wav.loadFromFile("What the dog doin.wav"); err != Error::None)
    // {
    //     Log::Add(Log::Level::Error, TAG, "Failed to load test wav file. Error: %d", ErrorToString(err));
    //     return;
    // }
    // robot.getAudioManager().getMixer().addAudioProvider(new WavProvider(wav));
}
#ifdef __cplusplus
}
#endif