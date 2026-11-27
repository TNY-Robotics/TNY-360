#include "ui/UIManager.hpp"
#include "ui/Menus.hpp"
#include "ui/Icons.hpp"
#include "common/Log.hpp"
#include "settings/Settings.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

UIManager::UIManager()
{

}

Status UIManager::init()
{
    screen = Screen::Create();
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

    LOG_INFO(TAG, "Marge minimale de pile avant caméra: %u octets", (unsigned)uxTaskGetStackHighWaterMark(nullptr));

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
    
    return Status::Ok;
}

Status UIManager::deinit()
{
    return Status::Ok;
}
