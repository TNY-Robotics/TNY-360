#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/Log.hpp"
#include "network/WiFiManager.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Bluetooth
{
    constexpr uint8_t MODULE_ID = 0x11;

    static ActionCallback actions[] = {
        
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}