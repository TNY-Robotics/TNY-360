#pragma once
#include "network/protocol/Protocol.hpp"
#include <esp_system.h>
#include "drivers/PowerDriver.hpp"

namespace Protocol
{
namespace Power
{
    constexpr uint8_t MODULE_ID = 0x09;

    static void GetVoltage(const RequestContext& ctx, const uint8_t* payload)
    {
        if (Error err = PowerDriver::ReadData(); err != Error::None) {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        PowerDriver::Data data = PowerDriver::GetData();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &data.voltage_v, sizeof(data.voltage_v));
    }

    static void GetCurrent(const RequestContext& ctx, const uint8_t* payload)
    {
        if (Error err = PowerDriver::ReadData(); err != Error::None) {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        PowerDriver::Data data = PowerDriver::GetData();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &data.current_a, sizeof(data.current_a));
    }

    static void GetPower(const RequestContext& ctx, const uint8_t* payload)
    {
        if (Error err = PowerDriver::ReadData(); err != Error::None) {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        PowerDriver::Data data = PowerDriver::GetData();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &data.power_w, sizeof(data.power_w));
    }


    static ActionCallback actions[] = {
        GetVoltage,     // 0x00
        GetCurrent,     // 0x01
        GetPower,       // 0x02
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}