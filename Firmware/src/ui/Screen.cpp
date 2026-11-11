#include "ui/Screen.hpp"
#include "settings/Settings.hpp"
#include "ui/ScreenSH.hpp"
#include "ui/ScreenSSD.hpp"

Screen* Screen::Create()
{
    // Determine which Screen to use based on settings
    ScreenType screen_type = Settings::GetConfig().screen.type;

    if (screen_type == ScreenType::SCREEN_SH1106)
    {
        return new ScreenSH();
    }
    else
    {
        return new ScreenSSD();
    }
    return nullptr;
}