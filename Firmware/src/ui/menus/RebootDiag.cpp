#include "ui/menus/RebootDiag.hpp"
#include "diagnostic/Diagnostic.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "esp_system.h"

MenuRebootDiag::MenuRebootDiag(Menu* parent)
    : Menu("Diagnostic", parent, Icons::DiagnosticMenu)
{
}

bool MenuRebootDiag::onBack()
{
    return false;
}

bool MenuRebootDiag::onSelect()
{
    isRebooting = true;
    return true;
}

bool MenuRebootDiag::onNext()
{
    return false;
}

bool MenuRebootDiag::onPrev()
{
    return false;
}


void MenuRebootDiag::onShow()
{
    
}

void MenuRebootDiag::onHide()
{ 
}

void MenuRebootDiag::onRender()
{
    if (isRebooting)
    {
        const char* text = "Rebooting ...";
        uint16_t width = Draw::GetTextWidth(text);
        Draw::Text(ScreenDriver::info.width / 2 - width / 2, HEADER_HEIGHT + 4, text);
        ScreenDriver::Upload(); // send data now

        Diagnostic::RebootInDiagnosticMode(); // Maybe we should read the return value and handle errors, but for now, fuck it.
    }
    else
    {
        renderHeader();
        {
            const char* text = "Reboot in       Diagnostic Mode?";
            Draw::Text(0, HEADER_HEIGHT + 4, text);
        }
        {
            const char* text = "Back";
            Draw::Text(12, ScreenDriver::info.height - 9, text);
        }
        {
            const char* text = "Reboot";
            uint16_t width = Draw::GetTextWidth(text);
            Draw::Text(ScreenDriver::info.width - width - 12, ScreenDriver::info.height - 9, text);
        }
        Draw::Blit(0, ScreenDriver::info.height - 9, 8, 8, (uint8_t*)Icons::ChevronLeft);
        Draw::Blit(ScreenDriver::info.width - 9, ScreenDriver::info.height - 9, 8, 8, (uint8_t*)Icons::ChevronRight);
    }
}

void MenuRebootDiag::onUpdate()
{
    triggerRender();
}
