#pragma once
#include "ui/Menus.hpp"

class MenuIMU : public Menus::Menu
{
public:
    MenuIMU() = default;
    MenuIMU(Menu* parent);
    virtual ~MenuIMU() = default;

    enum class Page: uint8_t
    {
        Accel = 0,
        Gyro,
        Visual,
        Count,
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
    Page currentPage = Page::Accel;
    int updateFreq = 10; // in Hz
    int updateCounter = 0;
};