#pragma once
#include "network/protocol/Protocol.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Protocol
{
    constexpr uint8_t MODULE_ID = 0x01;

    /** <API_REF>
     * @module protocol 0x01
     * @action setStreamFrequency 0x00
     * @desc Sets the frequency of the robot's data stream.
     * @arg frequency_hz uint16 Desired frequency of the data stream in Hz.
     * @impl done
     */
    static void SetStreamFrequency(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
     * @module protocol 0x01
     * @action setStreamFlags 0x01
     * @desc Sets the flags for the robot's data stream.
     * @arg flags uint8 Flags for the data stream.
     * @impl done
     */
    static void SetStreamFlags(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }


    static ActionCallback actions[] = {
        SetStreamFrequency,        // 0x00
        SetStreamFlags,            // 0x01
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}