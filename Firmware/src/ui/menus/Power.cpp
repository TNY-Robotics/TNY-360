#include "ui/menus/Power.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "common/config.hpp"
#include "drivers/PowerDriver.hpp"

MenuPower::MenuPower(Menu* parent)
    : Menu("Power", parent, Icons::PowerMenu)
{
}

bool MenuPower::onBack()
{
    return false;
}

bool MenuPower::onSelect()
{
    return false;
}

bool MenuPower::onNext()
{
    return false;
}

bool MenuPower::onPrev()
{
    return false;
}


void MenuPower::onShow()
{
}

void MenuPower::onHide()
{ 
}

void MenuPower::onRender()
{
    renderHeader();

    PowerDriver::ReadData();
    PowerDriver::Data data = PowerDriver::GetData();

    {
        char str[16+1];
        sprintf(str, "Voltage: %+1.1f V", data.voltage_v);
        Draw::Text(0, HEADER_HEIGHT + 4 + 12, str);
    }
    {
        char str[16+1];
        sprintf(str, "Current: %+1.1f A", data.current_a);
        Draw::Text(0, HEADER_HEIGHT + 4 + 27, str);
    }
    {
        char str[16+1];
        sprintf(str, "Power:   %+1.1f W", data.power_w);
        Draw::Text(0, HEADER_HEIGHT + 4 + 42, str);
    }
}

void MenuPower::onUpdate()
{
    if (updateCounter++ >= SCREEN_REFRESH_RATE / updateFreq)
    {
        updateCounter = 0;
        triggerRender();
    }
}
