#include "ui/UIManager.hpp"
#include "ui/Screen.hpp"
#include "ui/Menus.hpp"
#include "common/Log.hpp"

UIManager::UIManager()
{

}

Error UIManager::init()
{
    if (Error err = Screen::Init(); err != Error::None)
    {
        return err;
    }

    if (Error err = Menus::Init(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error UIManager::deinit()
{
    return Error::None;
}
