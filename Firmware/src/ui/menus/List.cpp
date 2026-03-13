#include "ui/menus/List.hpp"
#include "ui/Draw.hpp"
#include "common/config.hpp"

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
    ScreenDriver::Clear();

    if (m_selected_shift < MENU_LIST_ITEM_SELECTED_SHIFT) m_selected_shift += 1;

    m_view_shift_current += (m_selected_index - m_view_shift_current) * 0.25f;
    m_selected_index_current += (m_selected_index - m_selected_index_current) * 0.5f;

    const uint8_t text_height = 8;
    const uint8_t padding = 4;
    const uint8_t item_height = text_height + padding * 2;

    int16_t y_select_pos = ScreenDriver::info.height / 2 + (m_selected_index_current - m_view_shift_current) * item_height;
    Draw::RectFilled(0, y_select_pos - item_height / 2, ScreenDriver::info.width, item_height, ScreenDriver::COLOR_WHITE);
    
    for (uint8_t i = 0; i < m_item_count; i++)
    {
        bool selected = (i == m_selected_index);
        uint8_t x_pos = (selected ? m_selected_shift : MENU_LIST_ITEM_DEFAULT_SHIFT);
        uint8_t y_pos = ScreenDriver::info.height / 2 + (i - m_view_shift_current) * item_height;
        Draw::Text<true>(x_pos + 14, y_pos - text_height / 2, m_items[i]->getTitle(), selected ? ScreenDriver::COLOR_BLACK : ScreenDriver::COLOR_WHITE);
        Draw::RectRounded<true>(x_pos - 2, y_pos - 6, 12, 12, 2, ScreenDriver::COLOR_BLACK);
        Draw::Blit<true>(x_pos, y_pos - 4, 8, 8, (uint8_t*)m_items[i]->getIcon(), ScreenDriver::COLOR_WHITE, true);
    }

    renderHeader();

    ScreenDriver::Upload();
}

void MenuList::onUpdate()
{
    m_need_render = true; // always render (for smooth animations)
}