#pragma once
#include "menu.hpp"

class ResetMenu : public Menu
{
public:
    ResetMenu();
    ResetMenu(Menu* parent);
    virtual ~ResetMenu();

protected:
    bool m_resetting = false;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onRender() override;
    virtual void onUpdate() override;
};
