#include "ui/menus/Reboot.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "esp_system.h"

MenuReboot::MenuReboot(Menu* parent)
    : Menu("Reboot", parent, Icons::RebootMenu)
{
}

bool MenuReboot::onBack()
{
    return false;
}

bool MenuReboot::onSelect()
{
    isRebooting = true;
    return true;
}

bool MenuReboot::onNext()
{
    return false;
}

bool MenuReboot::onPrev()
{
    return false;
}


void MenuReboot::onShow()
{
    
}

void MenuReboot::onHide()
{ 
}

void MenuReboot::onRender()
{
    if (isRebooting)
    {
        const char* text = "Rebooting ...";
        uint16_t width = Draw::GetTextWidth(text);
        Draw::Text(Screen::GetInstance()->getInfo().width / 2 - width / 2, HEADER_HEIGHT + 4, text);
        Screen::GetInstance()->upload(); // send data now

        // Restart
        esp_restart();
    }
    else
    {
        renderHeader();
        {
            const char* text = "Are you sure?";
            uint16_t width = Draw::GetTextWidth(text);
            Draw::Text(Screen::GetInstance()->getInfo().width / 2 - width / 2, HEADER_HEIGHT + 4, text);
        }
        {
            const char* text = "Back";
            Draw::Text(12, Screen::GetInstance()->getInfo().height - 9, text);
        }
        {
            const char* text = "Reboot";
            uint16_t width = Draw::GetTextWidth(text);
            Draw::Text(Screen::GetInstance()->getInfo().width - width - 12, Screen::GetInstance()->getInfo().height - 9, text);
        }
        Draw::Blit(0, Screen::GetInstance()->getInfo().height - 9, 8, 8, (uint8_t*)Icons::ChevronLeft);
        Draw::Blit(Screen::GetInstance()->getInfo().width - 9, Screen::GetInstance()->getInfo().height - 9, 8, 8, (uint8_t*)Icons::ChevronRight);
    }
}

void MenuReboot::onUpdate()
{
    triggerRender();
}
