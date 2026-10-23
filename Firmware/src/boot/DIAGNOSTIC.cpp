#include "boot/BootManager.hpp"
#include "boot/MenuBootDiagnostic.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"
#include "common/I2C.hpp"
#include "common/NVS.hpp"
#include "Robot.hpp"
#include "ui/ScreenSH.hpp"
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
        if (Status err = Diagnostic::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize Diagnostic module");
            return true; // block robot from booting as normal
        }
        return Diagnostic::IsDiagnosticModeEnabled(); // boot in diag mode if flag enabled
        // return false;
    }

    void boot_DIAGNOSTIC()
    {
        // Initialize NetworkManager first, so we can use the web interface to display errors if needed
        NetworkManager man;
        if (Status err = man.init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize NetworkManager module");
            // TODO : Save error to display on screen if possible later
            return;
        }
        if (Status err = man.start(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to start NetworkManager module");
            // TODO : Save error to display on screen if possible later
            return;
        }

        // Initialize LED module for error display
        if (Status err = LED::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize LED module");
            // TODO : Save error to display on screen if possible later
            return;
        }
        
        UIManager& ui = Robot::GetInstance().getUIManager();
        if (Status err = ui.init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize UI module");
            // TODO : Save error to display on screen if possible later
            return;
        }

        // All done, display the diagnostic menu
        MenuBootDiagnostic menu(&man);
        Menus::SetCurrentMenu(&menu);

        // Infinite loop to keep the diagnostic menu alive
        while (true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
}