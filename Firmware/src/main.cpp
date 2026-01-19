#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "Robot.hpp"

static const char* TAG = "Main";

static Robot robot;

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    Log::Add(Log::Level::Info, TAG, "Starting robot ...");

    if (Error err = robot.init(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Robot Init failed with error: %d", static_cast<uint8_t>(err));
        return;
    }

    Log::Add(Log::Level::Info, TAG, "Robot initialized successfully.");

    Log::Add(Log::Level::Info, TAG, "Enabling body motors.");
    if (Error err = robot.getBody().enable(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to enable body motors. Error: %d", static_cast<uint8_t>(err));
        return;
    }

    Transformf stand_position{
        Vec3f{0.f, 0.f, 100.f},
        Quatf{}
    };
    if (Error err = robot.getBody().setPosture(stand_position); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to set body target position. Error: %d", static_cast<uint8_t>(err));
        return;
    }

    Log::Add(Log::Level::Info, TAG, "Robot is operational.");
}
#ifdef __cplusplus
}
#endif