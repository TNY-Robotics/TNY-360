#include "ui/menus/MotorCalib.hpp"
#include "ui/Icons.hpp"
#include "common/Log.hpp"
#include "ui/Draw.hpp"
#include "locomotion/Joint.hpp"

const char* jointId2Str(Joint::Id id)
{
    switch (id)
    {
        case Joint::Id::FrontLeftHipRoll: return "FL Hip Roll";
        case Joint::Id::FrontLeftHipPitch: return "FL Hip Pitch";
        case Joint::Id::FrontLeftKneePitch: return "FL Knee Pitch";
        case Joint::Id::BackLeftHipRoll: return "BL Hip Roll";
        case Joint::Id::BackLeftHipPitch: return "BL Hip Pitch";
        case Joint::Id::BackLeftKneePitch: return "BL Knee Pitch";
        case Joint::Id::BackRightHipRoll: return "BR Hip Roll";
        case Joint::Id::BackRightHipPitch: return "BR Hip Pitch";
        case Joint::Id::BackRightKneePitch: return "BR Knee Pitch";
        case Joint::Id::FrontRightHipRoll: return "FR Hip Roll";
        case Joint::Id::FrontRightHipPitch: return "FR Hip Pitch";
        case Joint::Id::FrontRightKneePitch: return "FR Knee Pitch";
        case Joint::Id::EarLeft: return "Ear Left";
        case Joint::Id::EarRight: return "Ear Right";
        default: return "Unknown Joint";
    }
}

MenuMotorCalib::MenuMotorCalib(Menu* parent)
    : Menu("Motors", parent, Icons::MotorMenu)
{
}

bool MenuMotorCalib::onBack()
{
    if (calibrating)
    {
        Joint* joint = Joint::GetJoint((Joint::Id) jointId);
        if (joint)
        {
            joint->getMotorController().stopCalibration();
            calibrating = false;
        }
        else LOG_ERROR("MenuMotorCalib", "Joint not found");
        triggerRender();
        return true;
    }
    triggerRender();
    return false;
}

bool MenuMotorCalib::onSelect()
{
    if (!calibrating)
    {
        Joint* joint = Joint::GetJoint((Joint::Id) jointId);
        if (joint)
        {
            joint->getMotorController().startCalibration();
            calibrating = true;
            triggerRender();
        }
        else LOG_ERROR("MenuMotorCalib", "Joint not found");
    }
    triggerRender();
    return true;
}

bool MenuMotorCalib::onNext()
{
    if (calibrating) return true;
    jointId = (jointId + 1) % (int) Joint::Id::Count;
    triggerRender();
    return false;
}

bool MenuMotorCalib::onPrev()
{
    if (calibrating) return true;
    jointId = (jointId - 1 + (int) Joint::Id::Count) % (int) Joint::Id::Count;
    triggerRender();
    return false;
}


void MenuMotorCalib::onShow()
{
    jointId = 0;
    calibrating = false;
}

void MenuMotorCalib::onHide()
{
    Joint* joint = Joint::GetJoint((Joint::Id) jointId);
    if (joint && joint->getMotorController().getCalibrationState() == MotorController::CalibrationState::CALIBRATING) 
    {
        joint->getMotorController().stopCalibration();
    }
}

void MenuMotorCalib::onRender()
{
    renderHeader();
    
    Joint* joint = Joint::GetJoint((Joint::Id) jointId);
    
    if (calibrating)
    {
        float progress = joint ? joint->getMotorController().getCalibrationProgress() : 0.f;
        Draw::Text(8, HEADER_HEIGHT + 8, "Calibrating...");
        Draw::RectRounded(8, HEADER_HEIGHT + 30, Screen::GetInstance()->getInfo().width - 16, 16, 4);
        Draw::RectRounded(9, HEADER_HEIGHT + 31, Screen::GetInstance()->getInfo().width - 18, 14, 3, Screen::COLOR_BLACK);
        Draw::RectRounded(9, HEADER_HEIGHT + 31, (Screen::GetInstance()->getInfo().width - 18) * progress, 14, 3);
    }
    else
    {
        const char* jointName = jointId2Str((Joint::Id) jointId);
        int width = Draw::GetTextWidth(jointName);
        Draw::Text((Screen::GetInstance()->getInfo().width - width) / 2, (Screen::GetInstance()->getInfo().height - 8) / 2, jointName);

        if (joint)
        {
            if (joint->getMotorController().getCalibrationState() == MotorController::CalibrationState::CALIBRATED)
            {
                Draw::Text(3*8, Screen::GetInstance()->getInfo().height - 8, "Calibrated");
            }
            else
            {
                Draw::Text(8, Screen::GetInstance()->getInfo().height - 8, "Not calibrated");
            }
        }
    }
}

void MenuMotorCalib::onUpdate()
{
    if (calibrating)
    {
        Joint* joint = Joint::GetJoint((Joint::Id) jointId);
        if (joint && joint->getMotorController().getCalibrationState() != MotorController::CalibrationState::CALIBRATING)
        {
            calibrating = false;
            triggerRender();
        }
    }

    if (++updateCounter >= updateFreq)
    {
        updateCounter = 0;
        triggerRender();
    }
}
