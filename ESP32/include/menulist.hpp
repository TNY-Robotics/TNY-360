#pragma once
#include "menu.hpp"

class MenuList : public Menu
{
public:
    MenuList();
    MenuList(const char* title);
    MenuList(const char* title, Menu* parent);
    MenuList(const char* title, Menu* parent, Menu** items, uint8_t item_count);
    virtual ~MenuList();

    void setItems(Menu** items, uint8_t item_count);

protected:
    uint8_t m_selected_index = 0;
    uint8_t m_view_shift = 0;
    uint8_t m_item_count = 0;

    Menu** m_items = nullptr;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onRender() override;
    virtual void onUpdate() override;
};
