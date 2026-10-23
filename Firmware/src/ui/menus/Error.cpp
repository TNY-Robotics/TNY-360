#include "ui/menus/Error.hpp"
#include "ui/Draw.hpp"
#include "common/LED.hpp"
#include "ui/Icons.hpp"
#include <stdio.h>

MenuError::MenuError()
    : Menu("Error", nullptr, Icons::ErrorMenu)
{
}

bool MenuError::onBack()
{
    LED::clearErrorCode();
    Menus::SetCurrentMenu(Menus::GetMenuFace());
    return true;
}

bool MenuError::onSelect()
{
    return false;
}

bool MenuError::onNext()
{
    return false;
}

bool MenuError::onPrev()
{
    return false;
}

void MenuError::onShow()
{
}

void MenuError::onHide()
{
}

void MenuError::onRender()
{
    Draw::Text(0, 0, "Woops! Error.");
    {
        char buffer[34];
        sprintf(buffer, "Module: 0x%02X", error.module);
        Draw::Text(0, 20, buffer);
    }
    {
        char buffer[34];
        sprintf(buffer, "Subsystem: 0x%02X", error.subsystem);
        Draw::Text(0, 30, buffer);
    }
    {
        char buffer[34];
        sprintf(buffer, "Code: 0x%02X", error.code);
        Draw::Text(0, 40, buffer);
    }
    {
        char buffer[34];
        sprintf(buffer, "Severity: 0x%02X", static_cast<uint8_t>(error.severity));
        Draw::Text(0, 50, buffer);
    }
}

void MenuError::onUpdate()
{
    
}

void MenuError::setErrorEvent(const Error::ErrorEvent& err)
{
    error = err;
}