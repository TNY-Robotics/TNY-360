#pragma once
#include <freertos/FreeRTOS.h>
#include "menu.hpp"
#include "wifi.hpp"

class WifiStatusMenu : public Menu
{
    static const TickType_t UPDATE_INTERVAL;

public:
    WifiStatusMenu();
    WifiStatusMenu(Menu* parent);
    virtual ~WifiStatusMenu();

protected:
    TickType_t m_last_update = 0;
    uint8_t m_selected = 0;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onRender() override;
    virtual void onUpdate() override;
};
