#pragma once
#include "modules/Menus.hpp"

class MenuI2C : public Menus::Menu
{
public:
    MenuI2C();
    MenuI2C(const char* title);
    MenuI2C(const char* title, Menu* parent);
    MenuI2C(const char* title, Menu* parent, const uint8_t icon[8]);
    virtual ~MenuI2C();

protected:
    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onCreate() override;
    virtual void onRender() override;
    virtual void onUpdate() override;

private:
    uint8_t m_i2c_bus_select = 1; // 0 = secondary, 1 = primary
    uint8_t m_devices_list[64] = {0};
    uint8_t m_device_count = 0;
    uint8_t m_addr_scan = 0;
    bool m_scanning = false;

    void startScan();
};