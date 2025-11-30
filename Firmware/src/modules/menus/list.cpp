#include "modules/menus/list.hpp"
#include "modules/Screen.hpp"
#include "config.hpp"

MenuList::MenuList()
    : Menu()
{
}

MenuList::MenuList(const char* title)
    : Menu(title)
{
}

MenuList::MenuList(const char* title, Menu* parent)
    : Menu(title, parent)
{
}

MenuList::MenuList(const char* title, Menu* parent, const uint8_t icon[8])
    : Menu(title, parent, icon)
{
}

MenuList::MenuList(const char* title, Menu* parent, Menu** items, uint8_t item_count)
    : Menu(title, parent)
{
    setItems(items, item_count);
}

MenuList::~MenuList()
{
    if (m_items)
    {
        delete[] m_items;
    }
}

void MenuList::setItems(Menu** items, uint8_t item_count)
{
    if (m_items)
    {
        delete[] m_items;
    }

    m_items = items;
    m_item_count = item_count;
}

void MenuList::onBack()
{
    if (parent)
    {
        Menus::SetCurrentMenu(parent);
        m_need_render = true;
    }
}

void MenuList::onSelect()
{
    if (m_selected_index < m_item_count)
    {
        Menus::SetCurrentMenu(m_items[m_selected_index]);
        m_need_render = true;

        // trick : set to negative shift to make it slide back when going back
        m_title_shift = -Menus::Menu::HEADER_TITLE_ANIMATION_SHIFT;
    }
}

void MenuList::onNext()
{
    if (m_selected_index < m_item_count - 1)
    {
        m_selected_index++;
        m_selected_shift = MENU_LIST_ITEM_DEFAULT_SHIFT;
    }
}

void MenuList::onPrev()
{
    if (m_selected_index > 0)
    {
        m_selected_index--;
        m_selected_shift = MENU_LIST_ITEM_DEFAULT_SHIFT;
    }
}

void MenuList::onCreate()
{
}

void MenuList::onRender()
{
    Screen::Clear();

    if (m_selected_shift < MENU_LIST_ITEM_SELECTED_SHIFT) m_selected_shift += 1;

    m_view_shift_current += (m_selected_index - m_view_shift_current) * 0.25f;
    m_selected_index_current += (m_selected_index - m_selected_index_current) * 0.5f;

    const uint8_t text_height = 8;
    const uint8_t padding = 4;
    const uint8_t item_height = text_height + padding * 2;

    int16_t y_select_pos = Screen::info.height / 2 + (m_selected_index_current - m_view_shift_current) * item_height;
    Draw::RectFilled(&Screen::info, 0, y_select_pos - item_height / 2, Screen::info.width, item_height, Screen::COLOR_WHITE);
    
    for (uint8_t i = 0; i < m_item_count; i++)
    {
        bool selected = (i == m_selected_index);
        uint8_t x_pos = (selected ? m_selected_shift : MENU_LIST_ITEM_DEFAULT_SHIFT);
        uint8_t y_pos = Screen::info.height / 2 + (i - m_view_shift_current) * item_height;
        Draw::Text<true>(&Screen::info, x_pos + 14, y_pos - text_height / 2, m_items[i]->getTitle(), selected ? Screen::COLOR_BLACK : Screen::COLOR_WHITE);
        Draw::RectRounded<true>(&Screen::info, x_pos - 2, y_pos - 6, 12, 12, 2, Screen::COLOR_BLACK);
        Draw::Blit<true>(&Screen::info, x_pos, y_pos - 4, 8, 8, (uint8_t*)m_items[i]->getIcon(), Screen::COLOR_WHITE, true);
    }

    renderHeader();

    Screen::Upload();
}

void MenuList::onUpdate()
{
    m_need_render = true; // always render (for smooth animations)
}
