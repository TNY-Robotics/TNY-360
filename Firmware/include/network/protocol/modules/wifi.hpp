#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/Log.hpp"
#include "network/WiFiManager.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace WiFi
{
    constexpr uint8_t MODULE_ID = 0x10;

    /** <API_REF>
     * @module wifi 0x10
     * @action connectToAP 0x00
     * @desc Connects the robot to a Wi-Fi access point.
     * @arg ssid string SSID of the Wi-Fi network.
     * @arg password string Password of the Wi-Fi network.
     * @impl done
     */
    static void ConnectToAP(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        char ssid[32];
        if (reader.readString(ssid, sizeof(ssid)) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        char password[64];
        if (reader.readString(password, sizeof(password)) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        Error err = Robot::GetInstance().getNetworkManager().getWiFiManager().connect(ssid, password);
        ctx.respond(err == Error::None ? ResponseStatus::Ok : ResponseStatus::InvalidParameters);
    }


    static ActionCallback actions[] = {
        ConnectToAP,           // 0x00
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}