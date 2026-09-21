#pragma once
#include "common/utils.hpp"
#include "drivers/CameraDriver.hpp"
#include "ui/Screen.hpp"
#include "ui/menus/Splash.hpp"

class UIManager
{
public:
    constexpr static const char* TAG = "UIManager";

    UIManager();

    /**
     * @brief Initialize the ui system.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the ui system.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    Screen& getScreen() { return *screen; }

    CameraDriver& getCamera() { return camera; }

private:
    Screen* screen = nullptr;
    CameraDriver camera;
};
