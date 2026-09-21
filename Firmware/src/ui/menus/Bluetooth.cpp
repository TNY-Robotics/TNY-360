#include "ui/menus/Bluetooth.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"

MenuBluetooth::MenuBluetooth(Menu* parent)
    : Menu("Bluetooth", parent, Icons::BluetoothMenu)
{
}

bool MenuBluetooth::onBack()
{
    return false;
}

bool MenuBluetooth::onSelect()
{
    return false;
}

bool MenuBluetooth::onNext()
{
    return false;
}

bool MenuBluetooth::onPrev()
{
    return false;
}


void MenuBluetooth::onShow()
{
}

void MenuBluetooth::onHide()
{ 
}

void MenuBluetooth::onRender()
{
    renderHeader();

    const char* text = "Not implemented";
    uint16_t width = Draw::GetTextWidth(text);
    Draw::Text(Screen::GetInstance()->getInfo().width / 2 - width / 2, Screen::GetInstance()->getInfo().height / 2 - 4, text);
}

void MenuBluetooth::onUpdate()
{
}
