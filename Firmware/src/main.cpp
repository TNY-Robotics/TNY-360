#include <freertos/FreeRTOS.h>
#include "common/Log.hpp"
#include "Robot.hpp"
#include "common/config.hpp"
#include "common/RPC.hpp"
#include "boot/BootManager.hpp"

static const char* TAG = "Main";

static Robot robot;
CameraDriver cam;
WiFiManager wifi;

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
        // just return to avoid booting as normal (should not happen anyway)
        return;
    }

    LOG_INFO(TAG, "Initializing robot (FIRMWARE_VERSION=%s) ...", FIRMWARE_VERSION);

    if (Status err = robot.init(); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Failed to initialize robot. Error : %s", StatusToString(err));
        return;
    }

    LOG_INFO(TAG, "Robot initialized successfully.");
    
    LOG_INFO(TAG, "Starting robot ...");

    if (Status err = robot.start(); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Failed to start robot. Error : %s", StatusToString(err));
        return;
    }

    LOG_INFO(TAG, "Robot started successfully !");

    // Nothing to do here

    LOG_INFO(TAG, ">>> Robot is operational.");
}
#ifdef __cplusplus
}
#endif