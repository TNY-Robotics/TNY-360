#pragma once
#include "ui/Menus.hpp"

class MenuSpeaker : public Menus::Menu
{
public:
    MenuSpeaker() = default;
    MenuSpeaker(Menu* parent);
    virtual ~MenuSpeaker() = default;

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
};