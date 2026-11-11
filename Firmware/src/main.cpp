#include "boot/BootManager.hpp"
#include "common/Log.hpp"
#include "System.Error.hpp"
#include "Robot.hpp"

static const char* TAG = "Main";

static Robot robot;

#ifdef __cplusplus
extern "C" {
#endif
void app_main()
{
    // Check for special boot state (zero-calibration, pending update, etc.)
    if (BootManager::CheckForSpecialBoot())
    {
        // Special boot state detected.
        // The corresponding boot will be handled by the BootManager.
        // wait forever to avoid booting as normal (should not happen anyway)
        return;
    }

    LOG_INFO(TAG, "Initializing robot (FIRMWARE_VERSION=%s) ...", FIRMWARE_VERSION);

    if (Status err = robot.init(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to initialize robot. Error : %s", StatusToString(err));
        Error::RegisterErrorEvent(System::ErrorEventInitFailed());
        return;
    }

    LOG_INFO(TAG, "Robot initialized successfully.");
    
    LOG_INFO(TAG, "Starting robot ...");

    if (Status err = robot.start(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to start robot. Error : %s", StatusToString(err));
        Error::RegisterErrorEvent(System::ErrorEventStartFailed());
        return;
    }

    LOG_INFO(TAG, "Robot started successfully !");

    // Nothing to do here

    LOG_INFO(TAG, ">>> Robot is operational.");
}
#ifdef __cplusplus
}
#endif