#include "menulist.hpp"
#include "screen.hpp"
#include "draw.hpp"

#define NB_DISPLAYED_ITEMS 3
#define ITEM_TEXT_PADDING 10

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
        set_current_menu(parent);
        m_need_render = true;
    }
}

void MenuList::onSelect()
{
    if (m_selected_index < m_item_count)
    {
        set_current_menu(m_items[m_selected_index]);
        m_need_render = true;
    }
}

void MenuList::onNext()
{
    if (m_selected_index < m_item_count - 1)
    {
        m_selected_index++;
        if (m_selected_index >= m_view_shift + NB_DISPLAYED_ITEMS)
        {
            m_view_shift++;
        }
    }
}

void MenuList::onPrev()
{
    if (m_selected_index > 0)
    {
        m_selected_index--;
        if (m_selected_index < m_view_shift)
        {
            m_view_shift--;
        }
    }
}

void MenuList::onRender()
{
    screen_info_t* screen_info = get_screen_info();

    screen_clear();
    renderHeader();

    uint8_t item_display_height = (screen_info->height - HEADER_HEIGHT-HEADER_PADDING) / NB_DISPLAYED_ITEMS;
    
    // submenus
    for (uint8_t i = 0; i < NB_DISPLAYED_ITEMS; i++)
    {
        if (m_view_shift + i < m_item_count)
        {
            uint16_t y = HEADER_HEIGHT+HEADER_PADDING + i * item_display_height;
            bool selected = m_view_shift + i == m_selected_index;
            draw_rect(screen_info, 0, y, screen_info->width, item_display_height, selected);
            draw_text(screen_info, ITEM_TEXT_PADDING, y+(item_display_height-8)/2, m_items[m_view_shift + i]->getTitle(), !selected);
        }
    }

    screen_upload();
}

void MenuList::onUpdate()
{
    
}
