#pragma once
#include "ui/Menus.hpp"
#include "ui/menus/Main.hpp"
#include "ui/Face.hpp"

class MenuFace : public Menus::Menu
{
public:
    static MenuFace* GetInstance() { return instance; }

    Face::EyesInfo eyes_info;

    MenuFace();
    virtual ~MenuFace();

    void setBehavior(Face::BehaviorFunction behavior);

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
    static MenuFace* instance;

    Face::BehaviorFunction m_behavior = nullptr;

    MenuMain menuMain;
};