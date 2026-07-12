#include "boot/BootManager.hpp"
#include "boot/MenuBootDiagnostic.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"
#include "common/I2C.hpp"
#include "common/NVS.hpp"
#include "ui/Menus.hpp"
#include "ui/Draw.hpp"
#include "ui/Icons.hpp"
#include "network/NetworkManager.hpp"
#include "diagnostic/Diagnostic.hpp"
#include <freertos/FreeRTOS.h>

namespace BootManager
{
    bool boot_DIAGNOSTIC_needed()
    {
        // Check for skip verification flag
        {
            NVS::Handle* nvsHandle;
            if (Status err = NVS::Open("boot", &nvsHandle); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Error opening NVS 'boot' namespace : %d", err);
                return true; // block robot from booting as normal
            }

            bool skip_diag = false;
            if (Status err = nvsHandle->get<bool>("skip_diag", skip_diag); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Error reading skip diagnostic flag from NVS : %d", err);
            }
            NVS::Close(nvsHandle);

            if (skip_diag)
            {
                LOG_DEBUG(TAG, "Diagnostic skip flag found. Skipping diagnostic boot.");
                return false; // skip diagnostic boot
            }
        }

        return true; // default to booting into diagnostic mode (if skip_diag is false or not set)
    }

    void boot_DIAGNOSTIC()
    {
        // Initialize LED module for error display
        if (Status err = LED::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize LED module");
            // Fuck, something is really wrong
            return;
        }

        // Initialize I2C for screen
        if (Status err = I2C::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize I2C module");
            // TODO : Display error on LED
            return;
        }

        // Initialize Screen module
        if (Status err = ScreenDriver::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize ScreenDriver module");
            // TODO : Display error on LED
            return;
        }
        // Initialize Menu system
        if (Status err = Menus::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize Menus module");
            // TODO : Display error on LED
            return;
        }

        // Display splash screen while booting
        Menus::SetCurrentMenu(Menus::GetMenuSplash());

        if (Status err = Diagnostic::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize Diagnostic module");
            // TODO : Display error on screen
            return;
        }

        // Verification mode is on the first boot, and we want to diagnose the robot
        Diagnostic::SetDiagnosticMode(true);

        NetworkManager man;
        if (Status err = man.init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize NetworkManager module");
            // TODO : Display error on screen
            return;
        }

        // All done, display the diagnostic menu
        MenuBootDiagnostic menu(&man);
        Menus::SetCurrentMenu(&menu);

        while (true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
}