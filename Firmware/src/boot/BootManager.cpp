#include "boot/BootManager.hpp"
#include "common/Log.hpp"
#include "common/NVS.hpp"

namespace BootManager
{
    bool CheckForSpecialBoot()
    {
        LOG_SCOPE(TAG, "BootManager::CheckForSpecialBoot");

        // Init NVS to check for flags
        if (Status err = NVS::Init(); err != Status::Ok)
        {
            // LOG_ERROR(TAG, "Error initializing NVS : %s", ErrorToString(err));
            return true; // block robot from booting as normal
        }

        if (boot_UPDATE_needed())
        {
            LOG_INFO(TAG, "Update is pending. Starting in UPDATE boot mode");
            boot_UPDATE();
            return true; // don't boot as normal
        }

        if (boot_VERIFICATION_needed())
        {
            LOG_INFO(TAG, "Verification data missing. Starting in VERIFICATION boot mode");
            boot_VERIFICATION();
            return true; // don't boot as normal
        }
        
        LOG_INFO(TAG, "No special boot needed. Starting normal boot");
        return false; // robot can proceed as normal
    }
}