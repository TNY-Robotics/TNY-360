#include "ui/menus/Tests.hpp"
#include "ui/Icons.hpp"

MenuTests::MenuTests(Menu* parent)
    : MenuList("Tests", parent, Icons::TestsMenu),
    menuI2C(MenuI2C(this)), menuIMU(MenuIMU(this)), menuPower(MenuPower(this)), menuSpeaker(MenuSpeaker(this)), menuMicrophone(MenuMicrophone(this))
{
    setItems({
        &menuI2C,
        &menuIMU,
        &menuPower,
        &menuSpeaker,
        &menuMicrophone,
    });
}
