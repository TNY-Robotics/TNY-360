#include "ui/UIManager.hpp"
#include "ui/ScreenSH.hpp"
#include "ui/ScreenSSD.hpp"
#include "ui/Menus.hpp"
#include "ui/Icons.hpp"
#include "common/Log.hpp"
#include "settings/Settings.hpp"

UIManager::UIManager()
{

}

Status UIManager::init()
{
    // get screen type from settings
    switch (Settings::GetConfig().screen.type)
    {
        case ScreenType::SCREEN_SH1106:
            screen = new ScreenSH();
            break;
        case ScreenType::SCREEN_SSD1309:
            screen = new ScreenSSD();
            break;
        default:
            LOG_ERROR(TAG, "Unknown screen type in settings. Disabling screen.");
            break;
    }

    if (screen)
    {
        if (Status err = screen->init(); err != Status::Ok)
        {
            return err;
        }

        if (Status err = Menus::Init(); err != Status::Ok)
        {
            return err;
        }
    }

    if (Status err = camera.init(); err == Status::Ok)
    {
        if (Status err = camera.start(); err != Status::Ok)
        {
            // not critical, we can still use the ui without camera features
            // don't return failure, just log the error
            LOG_DEBUG(TAG, "Camera start failed. Error : 0x%0x", err);
        }
    }
    else
    {
        // not critical, we can still use the ui without camera features
        // don't return failure, just log the error
        LOG_DEBUG(TAG, "Camera init failed. Error : 0x%0x", err);
    }
    
    // Display splash screen
    Menus::SetCurrentMenu(Menus::GetMenuSplash());

    return Status::Ok;
}

Status UIManager::deinit()
{
    return Status::Ok;
}
