#pragma once
#include "config.hpp"
#include "common/utils.hpp"

namespace Settings
{
    Status Init();
    Status Deinit();
    RobotConfig GetConfig();
    Status SaveAndReboot();
}