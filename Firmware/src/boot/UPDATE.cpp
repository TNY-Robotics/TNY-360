#include "boot/BootManager.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"
#include "common/I2C.hpp"
#include "common/NVS.hpp"
#include "network/WiFiManager.hpp"
#include "ui/Menus.hpp"
#include "ui/Draw.hpp"
#include "ui/Icons.hpp"
#include "drivers/ScreenDriver.hpp"
#include "network/UpdateManager.hpp"
#include "Robot.hpp"
#include <freertos/FreeRTOS.h>
#include <esp_ota_ops.h>

namespace BootManager
{
    bool boot_UPDATE_needed()
    {
        return Robot::GetInstance().getNetworkManager().getUpdateManager().isUpdatePending();
    }

    void boot_UPDATE()
    {
        // Initialize LED module for error display
        LOG_DEBUG(TAG, "Initializing LED");
        if (Status err = LED::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize LED module");
            esp_ota_mark_app_invalid_rollback_and_reboot();
            return;
        }

        // Initialize I2C for screen
        if (Status err = I2C::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize I2C module");
            esp_ota_mark_app_invalid_rollback_and_reboot();
            return;
        }

        // Initialize Screen and Menu system for user interface
        LOG_DEBUG(TAG, "Initializing ScreenDriver");
        if (Status err = ScreenDriver::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize ScreenDriver module");
            esp_ota_mark_app_invalid_rollback_and_reboot();
            return;
        }

        // Display a "Verifying firmware" message
        ScreenDriver::Clear();
        Draw::Text(32, 0, "Updating");
        Draw::Text(0, 16, "Verifying       firmware...");
        Draw::Text(0, 45, "Don't turn off  the robot.");
        ScreenDriver::Upload();

        UpdateManager& update_manager = Robot::GetInstance().getNetworkManager().getUpdateManager();

        // Continue the update process
        if (Status err = update_manager.continueUpdate(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to continue update process");
            return;
        }

        // If we reach this point, the update process is complete and successful
        ScreenDriver::Clear();
        Draw::Text(32, 0, "Updating");
        Draw::Text(0, 16, "Update complete!");
        Draw::Text(0, 55, "Rebooting...");
        ScreenDriver::Upload();
        LOG_INFO(TAG, "Update process completed successfully, rebooting...");
        // Small delay to ensure message is displayed before reboot
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
    }
}