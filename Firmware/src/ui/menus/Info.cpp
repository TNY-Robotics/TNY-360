#include "ui/menus/Info.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"

MenuInfo::MenuInfo(Menu* parent)
    : Menu("Info", parent, Icons::InfoMenu)
{
}

bool MenuInfo::onBack()
{
    return false;
}

bool MenuInfo::onSelect()
{
    return false;
}

bool MenuInfo::onNext()
{
    return false;
}

bool MenuInfo::onPrev()
{
    return false;
}


void MenuInfo::onShow()
{
}

void MenuInfo::onHide()
{ 
}

void MenuInfo::onRender()
{
    renderHeader();
    Draw::Text(0, HEADER_HEIGHT + 2, "Firmware version");
    Draw::Text(0, HEADER_HEIGHT + 12, FIRMWARE_VERSION);
}

void MenuInfo::onUpdate()
{
}
