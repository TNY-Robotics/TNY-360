#pragma once
#include "ui/Menus.hpp"
#include "ui/menus/List.hpp"
#include "ui/menus/I2C.hpp"
#include "ui/menus/IMU.hpp"
#include "ui/menus/Power.hpp"
#include "ui/menus/Sound.hpp"

class MenuTests : public MenuList
{
public:
    MenuTests() = default;
    MenuTests(Menu* parent);
    virtual ~MenuTests() = default;

private:
    MenuI2C menuI2C;
    MenuIMU menuIMU;
    MenuPower menuPower;
    MenuSound menuSound;
};