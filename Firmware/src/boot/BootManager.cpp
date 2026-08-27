#include "boot/BootManager.hpp"
#include "common/Log.hpp"
#include "common/NVS.hpp"
#include "settings/Settings.hpp"

namespace BootManager
{
    bool CheckForSpecialBoot()
    {
        LOG_SCOPE(TAG, "BootManager::CheckForSpecialBoot");

        // Initialize Settings system
        if (Status err = Settings::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize Settings module : %s", StatusToString(err));
            return true; // block robot from booting as normal
        }

        // Init NVS to check for flags
        if (Status err = NVS::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Error initializing NVS : %s", StatusToString(err));
            return true; // block robot from booting as normal
        }

        if (boot_DIAGNOSTIC_needed())
        {
            LOG_INFO(TAG, "Starting in DIAGNOSTIC boot mode");
            boot_DIAGNOSTIC();
            return true; // don't boot as normal
        }

        if (boot_UPDATE_needed())
        {
            LOG_INFO(TAG, "Starting in UPDATE boot mode");
            boot_UPDATE();
            return true; // don't boot as normal
        }
        
        LOG_INFO(TAG, "No special boot needed. Starting normal boot");
        return false; // robot can proceed as normal
    }
}