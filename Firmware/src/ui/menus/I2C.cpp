#include "ui/menus/I2C.hpp"
#include "ui/Draw.hpp"
#include "ui/Icons.hpp"
#include "common/config.hpp"
#include "common/I2C.hpp"
#include <cmath>

MenuI2C::MenuI2C(Menu* parent)
    : Menu("I2C Scan", parent, Icons::I2CMenu)
{
}

bool MenuI2C::onBack()
{
    if (parent)
    {
        Menus::SetCurrentMenu(parent);
        triggerRender();
    }
    return true;
}

bool MenuI2C::onSelect()
{
    triggerRender();
    startScan();
    return true;
}

bool MenuI2C::onNext()
{
    m_i2c_bus_select = (m_i2c_bus_select + 1) % 2;
    triggerRender();
    startScan();
    return true;
}

bool MenuI2C::onPrev()
{
    m_i2c_bus_select = (m_i2c_bus_select + 1) % 2;
    triggerRender();
    startScan();
    return true;
}

void MenuI2C::onShow()
{
    startScan();
}

void MenuI2C::onHide()
{
    m_scanning = false;
}

void MenuI2C::onRender()
{
    char i2c_bus_str[32];
    sprintf(i2c_bus_str, "Bus: %s", m_i2c_bus_select ? "Primary" : "Secondary");
    Draw::Text<true>(2, Menu::HEADER_HEIGHT + 4, i2c_bus_str, ScreenDriver::COLOR_WHITE);

    char i2c_scanning[32];
    if (m_scanning)
    {
        sprintf(i2c_scanning, "Scanning %3d/128", m_addr_scan);
    }
    else
    {
        sprintf(i2c_scanning, "Scan complete");
    }

    char i2c_devices_found[32];
    sprintf(i2c_devices_found, "Found: %d", m_device_count);

    Draw::Text<true>(2, Menu::HEADER_HEIGHT + 16, i2c_scanning, ScreenDriver::COLOR_WHITE);
    Draw::Text<true>(2, Menu::HEADER_HEIGHT + 28, i2c_devices_found, ScreenDriver::COLOR_WHITE);

    if (m_device_count > 0)
    {
        char device_addr_str[32];
        sprintf(device_addr_str, "Device: 0x%02X", m_devices_list[m_device_count - 1]);
        Draw::Text<true>(2, Menu::HEADER_HEIGHT + 40, device_addr_str, ScreenDriver::COLOR_WHITE);
    }

    renderHeader();
}

void MenuI2C::onUpdate()
{
    if (m_scanning)
    {
        triggerRender();
        if (m_addr_scan > 127)
        {
            m_scanning = false;
            return;
        }

        Status err = I2C::ProbeAddress(m_i2c_bus_select ? I2C::handle_primary : I2C::handle_secondary, m_addr_scan);
        if (err == Status::Ok)
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