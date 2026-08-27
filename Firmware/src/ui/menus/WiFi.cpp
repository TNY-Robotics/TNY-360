#include "ui/menus/WiFi.hpp"
#include "common/Log.hpp"
#include "ui/Icons.hpp"
#include "ui/Draw.hpp"
#include "Robot.hpp"

#include "qrcodegen.hpp"

MenuWiFi::MenuWiFi(Menu* parent)
    : Menu("WiFi", parent, Icons::WiFiMenu)
{
}

bool MenuWiFi::onBack()
{
    return false;
}

bool MenuWiFi::onSelect()
{
    return false;
}

bool MenuWiFi::onNext()
{
    qr_mode = !qr_mode;
    triggerRender();
    return true;
}

bool MenuWiFi::onPrev()
{
    qr_mode = !qr_mode;
    triggerRender();
    return true;
}


void MenuWiFi::onShow()
{
    qr_mode = false;
}

void MenuWiFi::onHide()
{ 
}

void MenuWiFi::onRender()
{
    WiFiManager& man = Robot::GetInstance().getNetworkManager().getWiFiManager();

    renderHeader();

    char status[17];
    sprintf(status, "%s %s", man.getMode() == WiFiManager::Mode::STA ? "STA" : "AP", WiFiManager::StateToString(man.getState()));
    Draw::Text(0, HEADER_HEIGHT + HEADER_PADDING, status);

    Draw::Text(0, HEADER_HEIGHT + HEADER_PADDING + 14, "SSID:");
    char ssid[12];
    strncpy(ssid, man.getSSID(), sizeof(ssid));
    ssid[sizeof(ssid) - 1] = '\0'; // Ensure null-termination
    Draw::Text(40, HEADER_HEIGHT + HEADER_PADDING + 14, ssid);
    
    Draw::Text(0, HEADER_HEIGHT + HEADER_PADDING + 28, "IP:");
    Draw::Text(24, HEADER_HEIGHT + HEADER_PADDING + 28, man.getIPAddr());

    // Draw::Text(86, 54, "Menu");
    // Draw::Blit(ScreenDriver::info.width - 9, ScreenDriver::info.height - 9, 8, 8, (uint8_t*)Icons::ChevronRight);
}

void MenuWiFi::onUpdate()
{
    if (counter++ > SCREEN_REFRESH_RATE)
    {
        counter = 0;
        triggerRender();
    }
}
