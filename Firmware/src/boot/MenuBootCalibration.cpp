#include "boot/MenuBootCalibration.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "common/NVS.hpp"
#include "drivers/MotorDriver.hpp"
#include "Robot.hpp"

constexpr float JOINT_ANGLE[(int) Joint::Id::Count] = {
    DEG_TO_RAD(0.f),  // FrontLeftHipRoll
    DEG_TO_RAD(-135.f),    // FrontLeftHipPitch
    DEG_TO_RAD(150.f),  // FrontLeftKneePitch
    DEG_TO_RAD(0.f),  // BackLeftHipRoll
    DEG_TO_RAD(-45.f),    // BackLeftHipPitch
    DEG_TO_RAD(150.f),  // BackLeftKneePitch
    DEG_TO_RAD(0.f),  // BackRightHipRoll
    DEG_TO_RAD(-45.f),    // BackRightHipPitch
    DEG_TO_RAD(150.f),  // BackRightKneePitch
    DEG_TO_RAD(0.f),  // FrontRightHipRoll
    DEG_TO_RAD(-135.f),    // FrontRightHipPitch
    DEG_TO_RAD(150.f),  // FrontRightKneePitch
    DEG_TO_RAD(0.f),    // EarLeft
    DEG_TO_RAD(0.f)     // EarRight
};

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
    if (page == Page::CalibMotor)
    {
        // Stop the motor calibration and go back to intro page
        Joint* joint = Joint::GetJoint(jointId);
        if (joint) joint->getMotorController().stopCalibration();
        MotorDriver::DisableAllMotors();
        page = Page::Intro;
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
    // reset all joints calibrations
    for (int i = 0; i < (int) Joint::Id::Count; i++)
    {
        Joint* joint = Joint::GetJoint((Joint::Id)i);
        if (joint)
        {
            joint->getMotorController().deleteCalibrationData(true);
        }
    }
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
                // Move the joint to its rest angle (to provide space for the other joints calibrations)
                if ((int)jointId < (int)Joint::Id::Count - 2) // -2 to skip ears
                {
                    LOG_DEBUG(TAG, "Motor calibration done for joint %d, moving to rest angle (%.1fdeg)", jointId, RAD_TO_DEG(JOINT_ANGLE[(int)jointId]));
                    MotorController& ctrl = joint->getMotorController();
                    float min_angle = joint->getMinAngle();
                    float max_angle = joint->getMaxAngle();
                    float rest_angle_progress = (JOINT_ANGLE[(int)jointId] - min_angle) / (max_angle - min_angle);
                    if (joint->isInverted()) rest_angle_progress = 1 - rest_angle_progress; // invert progress if joint is inverted
                    ctrl.setTargetPosition(rest_angle_progress);
                    ctrl.enable();
                    MotorDriver::SendData();
                    // wait a bit for the motor to move
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    // now disable it and move to the next one
                    ctrl.disable();
                    MotorDriver::DisableAllMotors();
                }

                // go to the next joint
                jointId = static_cast<Joint::Id>(static_cast<uint8_t>(jointId) + 1);

                if ((int)jointId == (int)Joint::Id::Count - 2) // -2 to skip ears
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
