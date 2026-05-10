#include "boot/MenuBootCalibration.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "common/NVS.hpp"
#include "drivers/MotorDriver.hpp"
#include "Robot.hpp"

bool MenuBootCalibration::onBack()
{
    if (page == Page::Intro)
    {
        // store in NVS boot that we skip calib
        NVS::Handle* nvsHandle;
        if (Error err = NVS::Open("boot", &nvsHandle); err == Error::None)
        {
            nvsHandle->set<bool>("skip_calib", true);
            delete nvsHandle;
            esp_restart(); // restart to apply skip flag
        }

        // Show done page
        page = Page::Done;
        triggerRender();
    }
    return true;
}

bool MenuBootCalibration::onSelect()
{
    if (page == Page::Intro)
    {
        page = Page::CalibMotor;
        jointId = (Joint::Id) 0;
        progress = 0;
        triggerRender();
    }
    return true;
}

bool MenuBootCalibration::onNext()
{
    return false;
}

bool MenuBootCalibration::onPrev()
{
    return false;
}


void MenuBootCalibration::onShow()
{
    triggerRender();
    Robot::GetInstance().getBody().init();
}

void MenuBootCalibration::onHide()
{ 
}

void MenuBootCalibration::onRender()
{
    switch (page)
    {
        case Page::Intro:
        {
            Draw::Text(20, 0, "Calibration");
            Draw::Text(0, 12, "Robot is not    calibrated yet.");
            Draw::Text(0, 32, "Start full      calibration ?");
            Draw::Text(ScreenDriver::info.width - 12 - 16, ScreenDriver::info.height - 9, "Ok");
            Draw::Text(11, ScreenDriver::info.height - 9, "Skip");
            Draw::Blit(ScreenDriver::info.width - 9, ScreenDriver::info.height - 9, 8, 8, (uint8_t*) Icons::ChevronRight);
            Draw::Blit(0, ScreenDriver::info.height - 9, 8, 8, (uint8_t*) Icons::ChevronLeft);
            break;
        }
        case Page::CalibMotor:
        {
            Joint* joint = Joint::GetJoint(jointId);
            if (joint && joint->getMotorController().getCalibrationState() == MotorController::CalibrationState::ERROR)
            {
                Draw::Text(16, 0, "Motor calib.");
                Draw::Text(0, 12, "Sorry, an error occurred during motor calib.");
                Draw::Text(0, 42, "Check wiring andtry again.");
                return;
            }

            Draw::Text(16, 0, "Motor calib.");
            {
                char str[16+1];
                sprintf(str, "Motor %u/14", static_cast<uint8_t>(jointId) + 1);
                int width = Draw::GetTextWidth(str);
                Draw::Text((ScreenDriver::info.width - width) / 2, 18, str);
            }
            Draw::RectRounded(8, HEADER_HEIGHT + 30, ScreenDriver::info.width - 16, 16, 4);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, ScreenDriver::info.width - 18, 14, 3, ScreenDriver::COLOR_BLACK);
            Draw::RectRounded(9, HEADER_HEIGHT + 31, (ScreenDriver::info.width - 18) * progress, 14, 3);
            break;
        }
        case Page::CalibIMU:
        {
            Draw::Text(24, 0, "IMU calib.");
            Draw::Text(0, 12, "Place robot on  flat surface.");
            {
                Draw::RectRounded(8, HEADER_HEIGHT + 30, ScreenDriver::info.width - 16, 16, 4);
                Draw::RectRounded(9, HEADER_HEIGHT + 31, ScreenDriver::info.width - 18, 14, 3, ScreenDriver::COLOR_BLACK);
                Draw::RectRounded(9, HEADER_HEIGHT + 31, (ScreenDriver::info.width - 18) * progress, 14, 3);
            }
            break;
        }
        case Page::Done:
        {
            Draw::Text(20, 0, "Calibration");
            Draw::Text(0, 12, "Calibrated !");
            Draw::Text(0, 32, "Restart robot toapply changes.");
            break;
        }
        default: break;
    }
}

void MenuBootCalibration::onUpdate()
{
    switch (page)
    {
        case Page::CalibMotor:
        {
            Joint* joint = Joint::GetJoint(jointId);
            if (joint == nullptr)
            {
                LOG_ERROR(TAG, "Invalid jointId %d", jointId);
                return;
            }
            MotorController& mCtrl = joint->getMotorController();
            motorCalibState = mCtrl.getCalibrationState();

            if (motorCalibState == MotorController::CalibrationState::UNCALIBRATED)
            {
                mCtrl.startCalibration();
            }
            else if (motorCalibState == MotorController::CalibrationState::CALIBRATING)
            {
                progress = mCtrl.getCalibrationProgress();
                triggerRender();
            }
            else if (motorCalibState == MotorController::CalibrationState::CALIBRATED)
            {
                jointId = static_cast<Joint::Id>(static_cast<uint8_t>(jointId) + 1);
                if (jointId == Joint::Id::Count)
                {
                    page = Page::CalibIMU;
                }
                triggerRender();
            }
            else triggerRender(); // to show error state
            break;
        }
        case Page::CalibIMU:
        {
            // TODO
            break;
        }
        default: break;
    }
}
