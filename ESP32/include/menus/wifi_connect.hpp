#pragma once
#include <freertos/FreeRTOS.h>
#include "menu.hpp"
#include "wifi.hpp"

class WifiConnectMenu : public Menu
{
    static const TickType_t UPDATE_INTERVAL;
    static const TickType_t DISPLAY_LAPSE;

public:
    WifiConnectMenu();
    WifiConnectMenu(Menu* parent);
    virtual ~WifiConnectMenu();

protected:
    TickType_t m_last_update = 0;
    bool m_connected = false;
    bool m_error = false;
    bool m_started = false;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onRender() override;
    virtual void onUpdate() override;
};
