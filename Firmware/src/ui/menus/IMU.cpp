#include "ui/menus/IMU.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "ui/Screen.hpp"
#include "common/config.hpp"
#include "Robot.hpp"

MenuIMU::MenuIMU(Menu* parent)
    : Menu("IMU", parent, Icons::IMUMenu)
{
}

bool MenuIMU::onBack()
{
    return false;
}

bool MenuIMU::onSelect()
{
    return false;
}

bool MenuIMU::onNext()
{
    uint8_t count = static_cast<uint8_t>(Page::Count);
    currentPage = static_cast<Page>((static_cast<uint8_t>(currentPage) + 1) % count);
    triggerRender();
    return true;
}

bool MenuIMU::onPrev()
{
    uint8_t count = static_cast<uint8_t>(Page::Count);
    currentPage = static_cast<Page>((static_cast<uint8_t>(currentPage) + count - 1) % count);
    triggerRender();
    return true;
}


void MenuIMU::onShow()
{
}

void MenuIMU::onHide()
{ 
}

void MenuIMU::onRender()
{
    renderHeader();

    IMU* imu = Robot::GetInstance().getBody().getIMU();
    if (imu == nullptr)
    {
        Draw::Text(0, Screen::GetInstance()->getInfo().height / 2 - 4, "No IMU Available");
        return;
    }

    switch (currentPage)
    {
        case Page::Accel:
        {
            Draw::Text(0, HEADER_HEIGHT + 4, "Accelerometer");
            {
                char str[16+1];
                sprintf(str, "X: %+1.2f m/s2", imu->getAcceleration().x);
                Draw::Text(0, HEADER_HEIGHT + 4 + 12, str);
            }
            {
                char str[16+1];
                sprintf(str, "Y: %+1.2f m/s2", imu->getAcceleration().y);
                Draw::Text(0, HEADER_HEIGHT + 4 + 12 * 2, str);
            }
            {
                char str[16+1];
                sprintf(str, "Z: %+1.2f m/s2", imu->getAcceleration().z);
                Draw::Text(0, HEADER_HEIGHT + 4 + 12 * 3, str);
            }
            break;
        }
        case Page::Gyro:
        {
            Draw::Text(0, HEADER_HEIGHT + 4, "Gyroscope");
            {
                char str[16+1];
                sprintf(str, "X: %+1.2f rad/s", imu->getAngularVelocity().x);
                Draw::Text(0, HEADER_HEIGHT + 4 + 12, str);
            }
            {
                char str[16+1];
                sprintf(str, "Y: %+1.2f rad/s", imu->getAngularVelocity().y);
                Draw::Text(0, HEADER_HEIGHT + 4 + 12 * 2, str);
            }
            {
                char str[16+1];
                sprintf(str, "Z: %+1.2f rad/s", imu->getAngularVelocity().z);
                Draw::Text(0, HEADER_HEIGHT + 4 + 12 * 3, str);
            }
            break;
        }
        default:
            break;
    }
}

void MenuIMU::onUpdate()
{
    if (updateCounter++ >= SCREEN_REFRESH_RATE / updateFreq)
    {
        updateCounter = 0;
        triggerRender();
    }
}
