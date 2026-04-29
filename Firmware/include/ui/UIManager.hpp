#pragma once
#include "common/utils.hpp"
#include "drivers/CameraDriver.hpp"
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
    Error init();

    /**
     * @brief Deinitialize the ui system.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    CameraDriver& getCamera() { return camera; }

private:
    CameraDriver camera;
};
