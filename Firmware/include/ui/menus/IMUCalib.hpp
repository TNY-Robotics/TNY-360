#pragma once
#include "ui/Menus.hpp"
#include "common/config.hpp"

class MenuIMUCalib : public Menus::Menu
{
public:
    MenuIMUCalib() = default;
    MenuIMUCalib(Menu* parent);
    virtual ~MenuIMUCalib() = default;

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
    int updateFreq = 10;
    int updateCounter = 0;
};