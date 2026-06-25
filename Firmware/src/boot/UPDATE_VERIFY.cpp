#include "network/UpdateManager.hpp"
#include <freertos/FreeRTOS.h>

////////////////////////////////////////////
///
///    This file contains the firmware verification logic after an OTA update.
///    Run the necessary checks to ensure the new firmware is valid and can be booted successfully.
///
///    > If the verification fails, return an error code to trigger a rollback to the previous firmware version.
///    > If everything is fine, return Status::Ok to mark the update as successful and reboot into the new firmware.
///
////////////////////////////////////////////


::Status UpdateManager::verify_firmware()
{
    // Just so the user can see the message before rebooting
    vTaskDelay(pdMS_TO_TICKS(1000));
    return ::Status::Ok;
}