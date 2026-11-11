#pragma once
#include "ui/Menus.hpp"
#include "ui/menus/List.hpp"
#include "ui/menus/MotorCalib.hpp"
#include "ui/menus/IMUCalib.hpp"

class MenuCalibration : public MenuList
{
public:
    MenuCalibration() = default;
    MenuCalibration(Menu* parent);
    virtual ~MenuCalibration() = default;

private:
    MenuMotorCalib menuMotorCalib;
    MenuIMUCalib menuIMUCalib;
};