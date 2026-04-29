#include "ui/UIManager.hpp"
#include "drivers/ScreenDriver.hpp"
#include "ui/Menus.hpp"
#include "ui/Draw.hpp"
#include "ui/Icons.hpp"
#include "common/Log.hpp"

UIManager::UIManager()
{

}

Error UIManager::init()
{
    if (Error err = ScreenDriver::Init(); err != Error::None)
    {
        ErrorHandle({ErrorCode::ScreenInitFailed, "Unable to initialize screen"});
        return err;
    }

    if (Error err = Menus::Init(); err != Error::None)
    {
        ErrorHandle({ErrorCode::MenusInitFailed, "Unable to initialize menus"});
        return err;
    }

    if (Error err = camera.init(); err != Error::None)
    {
        LOG_WARNING(TAG, "Camera initialization failed. Camera features will be unavailable. Error: %s", ErrorToString(err));
        // not critical, we can still use the ui without camera features
    }
    
    // Display splash screen
    Menus::SetCurrentMenu(Menus::GetMenuSplash());

    return Error::None;
}

Error UIManager::deinit()
{
    return Error::None;
}
