#include "modules/menus/i2c.hpp"
#include "modules/Screen.hpp"
#include "config.hpp"
#include "modules/I2C.hpp"
#include <cmath>

MenuI2C::MenuI2C()
    : Menu()
{
}

MenuI2C::MenuI2C(const char* title)
    : Menu(title)
{
}

MenuI2C::MenuI2C(const char* title, Menu* parent)
    : Menu(title, parent)
{
}

MenuI2C::MenuI2C(const char* title, Menu* parent, const uint8_t icon[8])
    : Menu(title, parent, icon)
{
}

MenuI2C::~MenuI2C()
{
}

void MenuI2C::onBack()
{
    if (parent)
    {
        Menus::SetCurrentMenu(parent);
        m_need_render = true;
    }
}

void MenuI2C::onSelect()
{
    m_need_render = true;
    startScan();
}

void MenuI2C::onNext()
{
    m_i2c_bus_select = (m_i2c_bus_select + 1) % 2;
    m_need_render = true;
    startScan();
}

void MenuI2C::onPrev()
{
    m_i2c_bus_select = (m_i2c_bus_select + 1) % 2;
    m_need_render = true;
    startScan();
}

void MenuI2C::onCreate()
{
    startScan();
}

void MenuI2C::onRender()
{
    Screen::Clear();

    char i2c_bus_str[32];
    sprintf(i2c_bus_str, "Bus: %s", m_i2c_bus_select ? "Primary" : "Secondary");
    Draw::Text<true>(&Screen::info, 2, Menu::HEADER_HEIGHT + 4, i2c_bus_str, Screen::COLOR_WHITE);

    char i2c_scanning[32];
    if (m_scanning)
    {
        sprintf(i2c_scanning, "Scanning %3d/255", m_addr_scan);
    }
    else
    {
        sprintf(i2c_scanning, "Scan complete");
    }

    char i2c_devices_found[32];
    sprintf(i2c_devices_found, "Found: %d", m_device_count);

    Draw::Text<true>(&Screen::info, 2, Menu::HEADER_HEIGHT + 16, i2c_scanning, Screen::COLOR_WHITE);
    Draw::Text<true>(&Screen::info, 2, Menu::HEADER_HEIGHT + 28, i2c_devices_found, Screen::COLOR_WHITE);

    if (m_device_count > 0)
    {
        char device_addr_str[32];
        sprintf(device_addr_str, "Device: 0x%02X", m_devices_list[m_device_count - 1]);
        Draw::Text<true>(&Screen::info, 2, Menu::HEADER_HEIGHT + 40, device_addr_str, Screen::COLOR_WHITE);
    }

    renderHeader();

    Screen::Upload();
}

void MenuI2C::onUpdate()
{
    if (m_scanning)
    {
        m_need_render = true;
        if (m_addr_scan > 127)
        {
            m_scanning = false;
            return;
        }

        Error err = I2C::ProbeAddress(m_i2c_bus_select ? I2C::handle_primary : I2C::handle_secondary, m_addr_scan);
        if (err == Error::Ok)
        {
            if (m_device_count < sizeof(m_devices_list))
            {
                m_devices_list[m_device_count++] = m_addr_scan;
            }
        }
        m_addr_scan++;
    }
}

void MenuI2C::startScan()
{
    m_scanning = true;
    m_addr_scan = 9; // skip reserved addresses
    m_device_count = 0;
    memset(m_devices_list, 0, sizeof(m_devices_list));
}