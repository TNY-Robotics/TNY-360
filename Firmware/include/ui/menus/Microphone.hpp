#pragma once
#include "ui/Menus.hpp"

class MenuMicrophone : public Menus::Menu
{
public:
    MenuMicrophone() = default;
    MenuMicrophone(Menu* parent);
    virtual ~MenuMicrophone() = default;

protected:
    virtual bool onBack() override;
    virtual bool onSelect() override;
    virtual bool onNext() override;
    virtual bool onPrev() override;

    virtual void onShow() override;
    virtual void onHide() override;
    virtual void onRender() override;
    virtual void onUpdate() override;
};