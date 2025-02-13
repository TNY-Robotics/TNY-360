#include <esp_log.h>
#include <string.h>
#include "menus/wifi_disconnect.hpp"
#include "wifi.hpp"
#include "screen.hpp"
#include "draw.hpp"
#include "storage.hpp"

const TickType_t WifiDisconnectMenu::UPDATE_INTERVAL = 100 / portTICK_PERIOD_MS;
const TickType_t WifiDisconnectMenu::DISPLAY_LAPSE = 1000 / portTICK_PERIOD_MS;

WifiDisconnectMenu::WifiDisconnectMenu(Menu* parent)
    : Menu("Disconnect", parent)
{
}

WifiDisconnectMenu::~WifiDisconnectMenu()
{
}

void WifiDisconnectMenu::onBack()
{
    if (parent)
    {
        set_current_menu(parent);
        m_need_render = true;
        m_started = false;
    }
}

void WifiDisconnectMenu::onSelect()
{
}

void WifiDisconnectMenu::onNext()
{
    
}

void WifiDisconnectMenu::onPrev()
{
    
}

void WifiDisconnectMenu::onRender()
{
    screen_info_t* screen_info = get_screen_info();

    screen_clear();
    renderHeader();

    const char* status_str = m_disconnected? "Disconnected" : m_error ? "Error" : "Disconnecting...";
    uint16_t str_width = draw_text_width(status_str);
    draw_text(screen_info, (screen_info->width-str_width)/2, screen_info->height/2, status_str, COLOR_WHITE);

    screen_upload();
}

void WifiDisconnectMenu::onUpdate()
{
    if (!m_started)
    {
        m_started = true;
        wifi_disconnect();
    }

    TickType_t now = xTaskGetTickCount();
    if ((now - m_last_update >= DISPLAY_LAPSE) && (m_disconnected || m_error))
    {
        this->onBack();
    }
    if (now - m_last_update >= UPDATE_INTERVAL && !(m_disconnected || m_error))
    {
        m_last_update = xTaskGetTickCount();
        wifi_status_t status = wifi_get_status();
        m_disconnected = status == WIFI_DISCONNECTED;
        m_error = status == WIFI_ERROR;
        m_need_render = true;
    }
}
