#pragma once
#include "network/protocol/Protocol.hpp"
#include <esp_system.h>
#include "drivers/PowerDriver.hpp"

namespace Protocol
{
namespace Power
{
    constexpr uint8_t MODULE_ID = 0x09;

    /** <API_REF>
     * @module power 0x09
     * @action getVoltage 0x00
     * @desc Gets the current voltage of the robot.
     * @result voltage_v float32 Current voltage in volts.
     * @impl done
     */
    static void GetVoltage(const RequestContext& ctx, const uint8_t* payload)
    {
        if (Status err = PowerDriver::ReadData(); err != Status::Ok) {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        PowerDriver::Data data = PowerDriver::GetData();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &data.voltage_v, sizeof(data.voltage_v));
    }

    /** <API_REF>
     * @module power 0x09
     * @action getCurrent 0x01
     * @desc Gets the current current of the robot.
     * @result current_a float32 Current current in amps.
     * @impl done
     */
    static void GetCurrent(const RequestContext& ctx, const uint8_t* payload)
    {
        if (Status err = PowerDriver::ReadData(); err != Status::Ok) {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        PowerDriver::Data data = PowerDriver::GetData();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &data.current_a, sizeof(data.current_a));
    }

    /** <API_REF>
     * @module power 0x09
     * @action getPower 0x02
     * @desc Gets the current power consumption of the robot.
     * @result power_w float32 Current power consumption in watts.
     * @impl done
     */
    static void GetPower(const RequestContext& ctx, const uint8_t* payload)
    {
        if (Status err = PowerDriver::ReadData(); err != Status::Ok) {
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