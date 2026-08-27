#include "ui/menus/Logs.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "common/Log.hpp"

MenuLogs::MenuLogs(Menu* parent)
    : Menu("Logs", parent, Icons::LogsMenu)
{
}

bool MenuLogs::onBack()
{
    return false;
}

bool MenuLogs::onSelect()
{
    return false;
}

bool MenuLogs::onNext()
{
    return false;
}

bool MenuLogs::onPrev()
{
    return false;
}


void MenuLogs::onShow()
{
}

void MenuLogs::onHide()
{ 
}

void MenuLogs::onRender()
{
    renderHeader();

    const uint16_t NB_LINES = 4;
    for (size_t i = 0; i < NB_LINES; i++)
    {
        uint16_t index = NB_LINES - i - 1;
        const Log::LineInfo& line = Log::GetLine(index);
        char str[128/8 + 1];
        snprintf(str, sizeof(str), "%.16s", line.message);
        Draw::Text(0, Menu::HEADER_HEIGHT + i * 12 + 4, str);
    }
}

void MenuLogs::onUpdate()
{
    // update every second to get new logs
    if (updateCounter++ > SCREEN_REFRESH_RATE / updateFrequency)
    {
        updateCounter = 0;
        triggerRender();
    }
}
