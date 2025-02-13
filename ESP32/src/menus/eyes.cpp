#include "menus/eyes.hpp"
#include "screen.hpp"
#include "draw.hpp"
#include "esp_random.h"

const uint8_t EyesMenu::EYES_SIZE = 10;

EyesMenu::EyesMenu()
    : Menu()
{
}

EyesMenu::~EyesMenu()
{
}

void EyesMenu::setMainMenu(Menu* main_menu)
{
    parent = main_menu; // note : using Menu::parent as pointer on main menu
}

void EyesMenu::onBack()
{
    if (parent) // note : using Menu::parent as pointer on main menu
    {
        set_current_menu(parent);
        m_need_render = true;
    }
}

void EyesMenu::onSelect()
{
    if (parent) // note : using Menu::parent as pointer on main menu
    {
        set_current_menu(parent);
        m_need_render = true;
    }
}

void EyesMenu::onNext()
{
}

void EyesMenu::onPrev()
{
}

void EyesMenu::onRender()
{
    screen_info_t* screen_info = get_screen_info();

    screen_clear();

    draw_circle(
        screen_info,
        screen_info->width / 4 + m_eyes_current_shift_x,
        screen_info->height / 2 + m_eyes_current_shift_y,
        m_eyes_size,
        COLOR_WHITE
    );

    draw_circle(
        screen_info,
        3 * screen_info->width / 4 + m_eyes_current_shift_x,
        screen_info->height / 2 + m_eyes_current_shift_y,
        m_eyes_size,
        COLOR_WHITE
    );

    screen_upload();
}

void EyesMenu::onUpdate()
{
    if (xTaskGetTickCount() - m_random_eyes_last_update > m_random_eyes_update_delay)
    {
        m_random_eyes_last_update = xTaskGetTickCount();
        m_random_eyes_update_delay = (500 + esp_random() % 3500) / portTICK_PERIOD_MS;
        m_eyes_target_shift_x = esp_random() % 10 - 5;
        m_eyes_target_shift_y = esp_random() % 10 - 5;
    }

    if (xTaskGetTickCount() - m_random_blink_last_update > m_random_blink_update_delay)
    {
        m_random_blink_last_update = xTaskGetTickCount();
        m_random_blink_update_delay = (1500 + esp_random() % 2500) / portTICK_PERIOD_MS;
        m_eyes_size = 0;
    }

    if (m_eyes_size != EYES_SIZE)
    {
        m_eyes_size += (EYES_SIZE - m_eyes_size) / 2;
        m_need_render = true;
    }

    if (m_eyes_current_shift_x != m_eyes_target_shift_x || m_eyes_current_shift_y != m_eyes_target_shift_y)
    {
        m_eyes_current_shift_x += (m_eyes_target_shift_x - m_eyes_current_shift_x) / 2;
        m_eyes_current_shift_y += (m_eyes_target_shift_y - m_eyes_current_shift_y) / 2;
        m_need_render = true;
    }
}
