#include "menus/reset.hpp"
#include "screen.hpp"
#include "draw.hpp"

ResetMenu::ResetMenu(Menu* parent)
    : Menu("Reset", parent)
{
}

ResetMenu::~ResetMenu()
{
}

void ResetMenu::onBack()
{
    if (parent)
    {
        set_current_menu(parent);
        m_need_render = true;
    }
}

void ResetMenu::onSelect()
{
    m_resetting = true;
}

void ResetMenu::onNext()
{
}

void ResetMenu::onPrev()
{
}

void ResetMenu::onRender()
{
    screen_info_t* screen_info = get_screen_info();

    screen_clear();
    renderHeader();

    const char* text = m_resetting ? "Resetting..." : "Reset ?";
    uint16_t str_width = draw_text_width(text);

    draw_text(screen_info, (screen_info->width - str_width) / 2, Menu::HEADER_HEIGHT + (screen_info->height - Menu::HEADER_HEIGHT) / 2, text, COLOR_WHITE);

    screen_upload();
}

void ResetMenu::onUpdate()
{
    if (m_resetting && !m_need_render)
    {
        // reset ... and then quit
        this->onBack();
    }
}
