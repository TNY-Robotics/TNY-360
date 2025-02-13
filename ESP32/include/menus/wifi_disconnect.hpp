#pragma once
#include <freertos/FreeRTOS.h>
#include "menu.hpp"
#include "wifi.hpp"

class WifiDisconnectMenu : public Menu
{
    static const TickType_t UPDATE_INTERVAL;
    static const TickType_t DISPLAY_LAPSE;

public:
    WifiDisconnectMenu();
    WifiDisconnectMenu(Menu* parent);
    virtual ~WifiDisconnectMenu();

protected:
    TickType_t m_last_update = 0;
    bool m_disconnected = false;
    bool m_error = false;
    bool m_started = false;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onRender() override;
    virtual void onUpdate() override;
};
