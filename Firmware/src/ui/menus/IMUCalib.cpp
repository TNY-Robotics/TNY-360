#include "ui/menus/IMUCalib.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "Robot.hpp"

MenuIMUCalib::MenuIMUCalib(Menu* parent)
    : Menu("IMU", parent, Icons::IMUMenu)
{
}

bool MenuIMUCalib::onBack()
{
    IMU* imu = Robot::GetInstance().getBody().getIMU();
    if (!imu) return false;

    if (imu->getCalibrationState() == IMU::CalibrationState::CALIBRATING)
    {
        imu->stopCalibration();
        triggerRender();
        return true;
    }

    return false;
}

bool MenuIMUCalib::onSelect()
{
    IMU* imu = Robot::GetInstance().getBody().getIMU();
    if (!imu) return false;

    if (imu->getCalibrationState() == IMU::CalibrationState::CALIBRATING)
    {
        return false;
    }

    // Disable motors, stop control loop, and then start calibration
    float delay = Robot::GetInstance().getBody().disableSmooth();
    vTaskDelay(pdMS_TO_TICKS(delay * 1000)); // Wait for smooth disable to complete
    if (Robot::GetInstance().getControlLoop().isRunning())
    {
        Robot::GetInstance().getControlLoop().stop();
    }
    imu->startCalibration(true);

    triggerRender();
    return true;
}

bool MenuIMUCalib::onNext()
{
    return false;
}

bool MenuIMUCalib::onPrev()
{
    return false;
}


void MenuIMUCalib::onShow()
{
    triggerRender();
}

void MenuIMUCalib::onHide()
{
}

void MenuIMUCalib::onRender()
{
    renderHeader();
    
    IMU* imu = Robot::GetInstance().getBody().getIMU();
    if (!imu) return;

    if (imu->getCalibrationState() == IMU::CalibrationState::CALIBRATING)
    {
        Draw::Text(8, HEADER_HEIGHT + HEADER_PADDING, "Calibrating...");
        Draw::RectRounded(8, HEADER_HEIGHT + HEADER_PADDING + 24, Screen::GetInstance()->getInfo().width - 16, 16, 5);
        Draw::RectRounded(10, HEADER_HEIGHT + HEADER_PADDING + 26, Screen::GetInstance()->getInfo().width - 20, 12, 3, Screen::COLOR_BLACK);
        Draw::RectRounded(10, HEADER_HEIGHT + HEADER_PADDING + 26, (Screen::GetInstance()->getInfo().width - 20) * imu->getCalibrationProgress(), 12, 3, Screen::COLOR_WHITE);
    }
    else
    {
        char buffer[16];
        switch (imu->getCalibrationState())
        {
            case IMU::CalibrationState::UNCALIBRATED: sprintf(buffer, "Uncalibrated"); break;
            case IMU::CalibrationState::CALIBRATED: sprintf(buffer, "Calibrated"); break;
            case IMU::CalibrationState::ERROR: sprintf(buffer, "Error"); break;
            default: sprintf(buffer, "Unknown"); break;
        }
        int width = Draw::GetTextWidth(buffer);
        Draw::Text((Screen::GetInstance()->getInfo().width - width) / 2, HEADER_HEIGHT + HEADER_PADDING + 24, buffer);
    }
}

void MenuIMUCalib::onUpdate()
{
    IMU* imu = Robot::GetInstance().getBody().getIMU();
    if (!imu) return;

    if (imu->getCalibrationState() != IMU::CalibrationState::CALIBRATING)
    {
        if (!Robot::GetInstance().getControlLoop().isRunning())
        {
            Robot::GetInstance().getControlLoop().start();
        }
    }

    if (updateCounter++ >= updateFreq)
    {
        updateCounter = 0;
        triggerRender();
    }
}
