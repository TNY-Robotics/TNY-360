#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/config.hpp"
#include "common/BinaryReader.hpp"
#include "ui/Face.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Face
{
    constexpr uint8_t MODULE_ID = 0x0B;

    /** <API_REF>
     * @module face 0x0B
     * @action sertEyesInfos 0x00
     * @desc Sets the current eyes infos for the robot's face.
     * @arg eyes_info Face::EyesInfo The eyes infos to set.
     * @impl done
     */
    static void SetEyesInfo(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        ::Face::EyesInfo eyes_info;
        if (Status s = reader.read(eyes_info); s != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Face::setEyesInfos(eyes_info);
        ctx.respond(ResponseStatus::Ok);
    }

    static ActionCallback actions[] = {
        SetEyesInfo,           // 0x00
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}