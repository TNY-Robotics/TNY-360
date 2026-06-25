#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/Log.hpp"
#include "common/Error.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Error
{
    constexpr uint8_t MODULE_ID = 0x13;

    /** <API_REF>
     * @module error 0x13
     * @action getErrorCount 0x00
     * @desc Gets the number of errors.
     * @result count uint16 The number of errors.
      * @impl done
     */
    static void GetErrorCount(const RequestContext& ctx, const uint8_t* payload)
    {
        uint16_t count = ::Error::GetErrorCount();
        ctx.respond(ResponseStatus::Ok, (uint8_t*)&count, sizeof(count));
    }

    /** <API_REF>
     * @module error 0x13
     * @action getErrorEventByIndex 0x01
     * @desc Gets the error event at the specified index.
     * @arg index uint16 The index of the error event to retrieve (0 is the most recent event).
     * @result event ErrorEvent The error event at the specified index.
      * @impl done
     */
    static void GetErrorEventByIndex(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint16_t index;
        if (reader.read(index) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        const ::Error::ErrorEvent& ev = ::Error::GetErrorEventByIndex(index);
        ctx.respond(ResponseStatus::Ok, (uint8_t*)&ev, sizeof(ev));
    }

    /** <API_REF>
     * @module error 0x13
     * @action getErrorEventById 0x02
     * @desc Gets the error event with the specified event ID.
     * @arg event_id uint16 The event ID of the error event to retrieve.
     * @result event ErrorEvent The error event with the specified event ID.
      * @impl done
     */
    static void GetErrorEventById(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint16_t eventId;
        if (reader.read(eventId) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        const ::Error::ErrorEvent& ev = ::Error::GetErrorEventById(eventId);
        ctx.respond(ResponseStatus::Ok, (uint8_t*)&ev, sizeof(ev));
    }

    static ActionCallback actions[] = {
        GetErrorCount,                // 0x00
        GetErrorEventByIndex,         // 0x01
        GetErrorEventById,            // 0x02
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}