#pragma once
#include "ui/Menus.hpp"

class MenuPower : public Menus::Menu
{
public:
    MenuPower() = default;
    MenuPower(Menu* parent);
    virtual ~MenuPower() = default;

    enum class Page: uint8_t
    {
        Accel = 0,
        Gyro = 1,
        Count = 2
    };

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
    int updateFreq = 10; // in Hz
    int updateCounter = 0;
};