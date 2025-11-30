#pragma once
#include "modules/Menus.hpp"

class MenuList : public Menus::Menu
{
public:
    MenuList();
    MenuList(const char* title);
    MenuList(const char* title, Menu* parent);
    MenuList(const char* title, Menu* parent, const uint8_t icon[8]);
    MenuList(const char* title, Menu* parent, Menu** items, uint8_t item_count);
    virtual ~MenuList();

    void setItems(Menu** items, uint8_t item_count);

protected:
    uint8_t m_selected_index = 0;
    uint8_t m_item_count = 0;
    uint8_t m_selected_shift = 4;

    float m_view_shift_current = 0.0f;
    float m_selected_index_current = 0.0f;

    Menu** m_items = nullptr;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onCreate() override;
    virtual void onRender() override;
    virtual void onUpdate() override;
};
