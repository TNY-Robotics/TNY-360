#include <esp_log.h>
#include <string.h>
#include "menus/wifi_connect.hpp"
#include "wifi.hpp"
#include "screen.hpp"
#include "draw.hpp"

const TickType_t WifiConnectMenu::UPDATE_INTERVAL = 100 / portTICK_PERIOD_MS;
const TickType_t WifiConnectMenu::DISPLAY_LAPSE = 1000 / portTICK_PERIOD_MS;

WifiConnectMenu::WifiConnectMenu(Menu* parent)
    : Menu("Connect", parent)
{
}

WifiConnectMenu::~WifiConnectMenu()
{
}

void WifiConnectMenu::onBack()
{
    if (parent)
    {
        set_current_menu(parent);
        m_need_render = true;
        m_started = false;
    }
}

void WifiConnectMenu::onSelect()
{
}

void WifiConnectMenu::onNext()
{
    
}

void WifiConnectMenu::onPrev()
{
    
}

void WifiConnectMenu::onRender()
{
    screen_info_t* screen_info = get_screen_info();

    screen_clear();
    renderHeader();

    const char* status_str = m_connected? "Connected" : m_error ? "Error" : "Connecting...";
    uint16_t str_width = draw_text_width(status_str);
    draw_text(screen_info, (screen_info->width-str_width)/2, screen_info->height/2, status_str, COLOR_WHITE);

    screen_upload();
}

void WifiConnectMenu::onUpdate()
{
    if (!m_started)
    {
        m_started = true;
        esp_err_t err = wifi_connect();
        if (err != ESP_OK)
        {
            ESP_LOGE("wifi_connect", "Error: %d", err);
            m_error = true;
        }
    }

    TickType_t now = xTaskGetTickCount();
    if ((now - m_last_update >= DISPLAY_LAPSE) && (m_connected || m_error))
    {
        this->onBack();
    }
    
    if (now - m_last_update >= UPDATE_INTERVAL && !(m_connected || m_error))
    {
        m_last_update = xTaskGetTickCount();
        wifi_status_t status = wifi_get_status();
        m_connected = status == WIFI_CONNECTED;
        m_error = status == WIFI_ERROR;
        m_need_render = true;
    }
}
