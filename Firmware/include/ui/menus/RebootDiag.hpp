#pragma once
#include "ui/Menus.hpp"

class MenuRebootDiag : public Menus::Menu
{
public:
    MenuRebootDiag() = default;
    MenuRebootDiag(Menu* parent);
    virtual ~MenuRebootDiag() = default;

protected:
    virtual bool onBack() override;
    virtual bool onSelect() override;
    virtual bool onNext() override;
    virtual bool onPrev() override;

    virtual void onShow() override;
    virtual void onHide() override;
    virtual void onRender() override;
    virtual void onUpdate() override;

private:
    bool isRebooting = false;
};