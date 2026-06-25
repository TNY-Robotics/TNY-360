#include "boot/BootManager.hpp"
#include "common/Log.hpp"
#include "common/LED.hpp"
#include "common/I2C.hpp"
#include "common/NVS.hpp"
#include "ui/Menus.hpp"
#include "ui/Draw.hpp"
#include "ui/Icons.hpp"
#include "boot/MenuBootCalibration.hpp"
#include "drivers/ScreenDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include "locomotion/MotorController.hpp"
#include <freertos/FreeRTOS.h>

namespace BootManager
{
    bool boot_CALIBRATION_needed()
    {
        // Check for skip calibration flag
        {
            NVS::Handle* nvsHandle;
            if (Status err = NVS::Open("boot", &nvsHandle); err != Status::Ok)
            {
                // LOG_ERROR(TAG, "Error opening NVS 'boot' namespace : %s", ErrorToString(err));
                return true; // block robot from booting as normal
            }

            bool skip_calib = false;
            if (Status err = nvsHandle->get<bool>("skip_calib", skip_calib); err != Status::Ok)
            {
                // LOG_ERROR(TAG, "Error reading skip calibration flag from NVS : %s", ErrorToString(err));
                NVS::Close(nvsHandle);
                return true; // block robot from booting as normal
            }
            NVS::Close(nvsHandle);

            if (skip_calib)
            {
                LOG_DEBUG(TAG, "Calibration skip flag found. Skipping calibration boot.");
                return false; // skip calibration boot
            }
        }

        // Check motor controller calibration data
        constexpr int NB_MOTORS = 14; // 12 for legs + 2 for ears
        for (int i = 0; i < NB_MOTORS; i++)
        {
            char key[32];
            sprintf(key, "MtrCtrl-%d", static_cast<int>(i));
            NVS::Handle* nvsHandle;
            if (Status err = NVS::Open(key, &nvsHandle); err != Status::Ok)
            {
                if (err == Status::NotFound) // not found ? means not calibrated yet
                {
                    LOG_INFO(TAG, "Couldn't open namespace for motor %d. Starting in CALIBRATION boot mode", i);
                    return true; // start calibration boot
                }
                else // something went wrong
                {
                    // LOG_ERROR(TAG, "Error opening NVS 'MtrCtrl-%d' namespace : %s", i, ErrorToString(err));
                    return true; // start calibration boot to allow user to fix the issue
                }
            }
            MotorController::CalibrationData calib_data;
            if (Status err = nvsHandle->get("calib_data", calib_data); err != Status::Ok)
            {
                if (err == Status::NotFound)
                {
                    LOG_INFO(TAG, "Calibration data not found for motor %d. Starting in CALIBRATION boot mode", i);
                    return true; // start calibration boot
                }
                else
                {
                    // LOG_ERROR(TAG, "Error occurred while fetching calibration data for motor %d : %s", i, ErrorToString(err));
                    return true; // start calibration boot to allow user to fix the issue
                }
            }
        }

        // Found ? Ok, check for IMU Calibration data
        // NVS::Handle* nvsHandle;
        // TODO

        return false;
    }

    void boot_CALIBRATION()
    {
        // Initialize LED module for error display
        if (Status err = LED::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize LED module");
            return;
        }

        // Initialize I2C for screen and motor driver
        if (Status err = I2C::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize I2C module");
            return;
        }

        // Initialize Screen and Menu system for user interface
        if (Status err = ScreenDriver::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize ScreenDriver module");
            // LED::LoopErrorCode(ErrorCode::ScreenInitFailed);
            return;
        }
        if (Status err = Menus::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize Menus module");
            // LED::LoopErrorCode(ErrorCode::ScreenInitFailed);
            return;
        }

        // Display splash screen
        Menus::SetCurrentMenu(Menus::GetMenuSplash());

        // Initialize Motor Driver for motor control
        if (Status err = MotorDriver::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize MotorDriver module");
            // LED::LoopErrorCode(ErrorCode::DriverInitFailed);
            return;
        }

        // Show calibration menu
        MenuBootCalibration menu;
        Menus::SetCurrentMenu(&menu);

        // infinite loop to keep menu variable alive
        while (true) { vTaskDelay(pdMS_TO_TICKS(100)); };
    }
}