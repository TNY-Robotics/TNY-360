#include <esp_log.h>
#include <string.h>
#include "menus/wifi_status.hpp"
#include "wifi.hpp"
#include "screen.hpp"
#include "draw.hpp"
#include "storage.hpp"

const TickType_t WifiStatusMenu::UPDATE_INTERVAL = 1000 / portTICK_PERIOD_MS;

WifiStatusMenu::WifiStatusMenu(Menu* parent)
    : Menu("Wifi Status", parent)
{
}

WifiStatusMenu::~WifiStatusMenu()
{
}

void WifiStatusMenu::onBack()
{
    if (parent)
    {
        set_current_menu(parent);
        m_need_render = true;
        m_selected = 0;
    }
}

void WifiStatusMenu::onSelect()
{
}

void WifiStatusMenu::onNext()
{
    m_selected = (m_selected + 1) % 3;
    m_need_render = true;
}

void WifiStatusMenu::onPrev()
{
    m_selected = (m_selected + 2) % 3;
    m_need_render = true;
}

void WifiStatusMenu::onRender()
{
    screen_info_t* screen_info = get_screen_info();

    screen_clear();
    renderHeader();
    
    wifi_status_t status = wifi_get_status();
    char* ssid = wifi_get_ssid();
    uint32_t ip = wifi_get_ip();

    const char* status_str = 
        status == WIFI_CONNECTED ? "Connected" :
        status == WIFI_DISCONNECTED ? "Disconnected" :
        status == WIFI_CONNECTING ? "Connecting" :
        status == WIFI_DISCONNECTING ? "Disconnecting" :
        status == WIFI_ERROR ? "Error" : "Unknown";

    char ip_str[16];
    sprintf(ip_str, "%ld.%ld.%ld.%ld", (ip >> 0) & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);

    char stat_name[16];
    char stat_value[16];

    switch (m_selected)
    {
    case 0:
        strncpy(stat_name, "Status", sizeof(stat_name));
        strncpy(stat_value, status_str, sizeof(stat_value));
        break;
    case 1:
        strncpy(stat_name, "SSID", sizeof(stat_name));
        strncpy(stat_value, ssid[0]? ssid: "Unknown", sizeof(stat_value));
        break;
    case 2:
        strncpy(stat_name, "IP", sizeof(stat_name));
        strncpy(stat_value, ip? ip_str: "Unknown", sizeof(stat_value));
        break;
    }

    uint16_t str_width = draw_text_width(stat_name);
    draw_text(screen_info, (screen_info->width - str_width) / 2, screen_info->height / 2 - 5, stat_name, COLOR_WHITE);
    str_width = draw_text_width(stat_value);
    draw_text(screen_info, (screen_info->width - str_width) / 2, screen_info->height / 2 + 5, stat_value, COLOR_WHITE);

    screen_upload();
}

void WifiStatusMenu::onUpdate()
{
    if (xTaskGetTickCount() - m_last_update >= UPDATE_INTERVAL)
    {
        m_last_update = xTaskGetTickCount();
        m_need_render = true;
    }
}
