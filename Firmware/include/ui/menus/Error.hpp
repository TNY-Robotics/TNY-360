#pragma once
#include "ui/Menus.hpp"

class MenuError : public Menus::Menu
{
public:
    MenuError();
    virtual ~MenuError() = default;

    // void setError(ErrorStruct::ErrorStruct err);

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
    // ErrorStruct::ErrorStruct error;
};