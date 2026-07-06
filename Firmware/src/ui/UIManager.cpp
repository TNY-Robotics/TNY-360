#include "ui/UIManager.hpp"
#include "drivers/ScreenDriver.hpp"
#include "ui/Menus.hpp"
#include "ui/Draw.hpp"
#include "ui/Icons.hpp"
#include "common/Log.hpp"

UIManager::UIManager()
{

}

Status UIManager::init()
{
    if (Status err = ScreenDriver::Init(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = Menus::Init(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = camera.init(); err != Status::Ok)
    {
        // not critical, we can still use the ui without camera features
    }

    if (Status err = camera.start(); err != Status::Ok)
    {
        // not critical, we can still use the ui without camera features
    }
    
    // Display splash screen
    Menus::SetCurrentMenu(Menus::GetMenuSplash());

    return Status::Ok;
}

Status UIManager::deinit()
{
    return Status::Ok;
}
