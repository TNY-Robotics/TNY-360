#include <esp_log.h>
#include <esp_system.h>
#include "menus/factory_reset.hpp"
#include "screen.hpp"
#include "draw.hpp"
#include "storage.hpp"

FactoryResetMenu::FactoryResetMenu(Menu* parent)
    : Menu("Factory Reset", parent)
{
}

FactoryResetMenu::~FactoryResetMenu()
{
}

void FactoryResetMenu::onBack()
{
    if (parent)
    {
        set_current_menu(parent);
        m_need_render = true;
    }
}

void FactoryResetMenu::onSelect()
{
    m_resetting = true;
}

void FactoryResetMenu::onNext()
{
}

void FactoryResetMenu::onPrev()
{
}

void FactoryResetMenu::onRender()
{
    screen_info_t* screen_info = get_screen_info();

    screen_clear();
    renderHeader();

    if (m_reset_error)
    {
        const char* text = "Error resetting";
        uint16_t str_width = draw_text_width(text);
        draw_text(screen_info, (screen_info->width - str_width) / 2, (screen_info->height - Menu::HEADER_HEIGHT) / 2, text, COLOR_WHITE);
    }
    else if (m_resetting)
    {
        const char* text = "Resetting...";
        uint16_t str_width = draw_text_width(text);
        draw_text(screen_info, (screen_info->width - str_width) / 2, (screen_info->height - Menu::HEADER_HEIGHT) / 2, text, COLOR_WHITE);
    }
    else
    {
        const char* text = "Are you sure ?";
        uint16_t str_width = draw_text_width(text);
        draw_text(screen_info, (screen_info->width - str_width) / 2, (screen_info->height - Menu::HEADER_HEIGHT) / 2, text, COLOR_WHITE);
    }

    screen_upload();
}

void FactoryResetMenu::onUpdate()
{
    if (m_resetting && !m_need_render)
    {
        if (storage_erase_all() == ESP_OK)
        {
            esp_restart();
        }
        else
        {
            m_reset_error = true;
            m_resetting = false;
        }
    }
}
