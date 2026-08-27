#pragma once
#include "ui/Menus.hpp"
#include "ui/menus/List.hpp"
#include "ui/menus/Info.hpp"
#include "ui/menus/Logs.hpp"
#include "ui/menus/Update.hpp"
#include "ui/menus/Reboot.hpp"
#include "ui/menus/RebootDiag.hpp"
#include "ui/menus/Reset.hpp"

class MenuSystem : public MenuList
{
public:
    MenuSystem() = default;
    MenuSystem(Menu* parent);
    virtual ~MenuSystem() = default;

private:
    MenuInfo menuInfo;
    MenuLogs menuLogs;
    MenuUpdate menuUpdate;
    MenuReboot menuReboot;
    MenuRebootDiag menuRebootDiag;
    MenuReset menuReset;
};