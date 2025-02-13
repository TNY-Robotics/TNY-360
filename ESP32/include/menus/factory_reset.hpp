#pragma once
#include "menu.hpp"

class FactoryResetMenu : public Menu
{
public:
    FactoryResetMenu();
    FactoryResetMenu(Menu* parent);
    virtual ~FactoryResetMenu();

protected:
    bool m_resetting = false;
    bool m_reset_error = false;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onRender() override;
    virtual void onUpdate() override;
};
