#include "ui/menus/Tests.hpp"
#include "ui/Icons.hpp"

MenuTests::MenuTests(Menu* parent)
    : MenuList("Tests", parent, Icons::TestsMenu),
    menuI2C(MenuI2C(this)), menuIMU(MenuIMU(this)), menuPower(MenuPower(this)), menuSound(MenuSound(this))
{
    setItems({
        &menuI2C,
        &menuIMU,
        &menuPower,
        &menuSound
    });
}
