#include "ui/menus/Calibration.hpp"
#include "ui/Icons.hpp"

MenuCalibration::MenuCalibration(Menu* parent)
    : MenuList("Calibration", parent, Icons::CalibrationMenu),
    menuMotorCalib(MenuMotorCalib(this)), menuIMUCalib(MenuIMUCalib(this))
{
    setItems({
        &menuMotorCalib,
        &menuIMUCalib
    });
}
