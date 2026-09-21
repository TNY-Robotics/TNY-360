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

MenuList::MenuList(const char* title, Menu* parent, const uint8_t icon[8], std::vector<Menu*> items)
    : Menu(title, parent, icon)
{
    setItems(items);
}

void MenuList::setItems(std::vector<Menu*> items)
{
    m_items = items;
}

bool MenuList::onBack()
{
    return false;
}

bool MenuList::onSelect()
{
    if (m_selected_index < m_items.size())
    {
        Menus::SetCurrentMenu(m_items[m_selected_index]);
    }
    return true;
}

bool MenuList::onNext()
{
    if (m_selected_index < m_items.size() - 1)
    {
        m_selected_index++;
        m_selected_shift = MENU_LIST_ITEM_DEFAULT_SHIFT;
    }
    return true;
}

bool MenuList::onPrev()
{
    if (m_selected_index > 0)
    {
        m_selected_index--;
        m_selected_shift = MENU_LIST_ITEM_DEFAULT_SHIFT;
    }
    return true;
}

void MenuList::onShow()
{
    m_selected_index_current = 0;
    m_selected_index = 0;
    m_view_shift_current = 0;
}

void MenuList::onHide()
{
}

void MenuList::onRender()
{
    if (m_selected_shift < MENU_LIST_ITEM_SELECTED_SHIFT) m_selected_shift += 1;

    m_view_shift_current += (m_selected_index - m_view_shift_current) * 0.25f;
    m_selected_index_current += (m_selected_index - m_selected_index_current) * 0.5f;

    const uint8_t text_height = 8;
    const uint8_t padding = 4;
    const uint8_t item_height = text_height + padding * 2 + 1;

    int16_t y_select_pos = Screen::GetInstance()->getInfo().height / 2 + (m_selected_index_current - m_view_shift_current) * item_height;
    Draw::RectRounded<true>(0, y_select_pos - item_height / 2, Screen::GetInstance()->getInfo().width, item_height, 4, Screen::COLOR_WHITE);
    Draw::RectRounded<true>(1, y_select_pos - item_height / 2 + 1, Screen::GetInstance()->getInfo().width-3, item_height-3, 2, Screen::COLOR_BLACK);
    
    for (uint8_t i = 0; i < m_items.size(); i++)
    {
        bool selected = (i == m_selected_index);
        int x_pos = (selected ? m_selected_shift : MENU_LIST_ITEM_DEFAULT_SHIFT);
        int y_pos = Screen::GetInstance()->getInfo().height / 2 + (i - m_view_shift_current) * item_height;
        if (y_pos < 0) continue;
        Draw::Text<true>(x_pos + 14, y_pos - text_height / 2, m_items[i]->getTitle());
        Draw::RectRounded<true>(x_pos - 2, y_pos - 6, 12, 12, 2, Screen::COLOR_BLACK);
        Draw::Blit<true>(x_pos, y_pos - 4, 8, 8, (uint8_t*)m_items[i]->getIcon(), Screen::COLOR_WHITE, true);
    }
    
    renderHeader();
}

void MenuList::onUpdate()
{
    triggerRender();
}