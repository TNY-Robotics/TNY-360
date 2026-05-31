#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/RPC.hpp"
#include "locomotion/Leg.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Leg
{
    constexpr uint8_t MODULE_ID = 0x04;

    static void SetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Error::None || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        uint8_t enabledFlag;
        if (reader.read(enabledFlag) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Error>([enabledFlag, legId](){
            Error err;
            for (int i = 0; i < (int) ::Leg::JointId::Count; i++)
            {
                bool isEnabled = (enabledFlag & (1 << i)) != 0;
                Joint& joint = Robot::GetInstance().getBody().getLeg((::Leg::Id)legId).getJoint((::Leg::JointId)i);
                if (isEnabled) err = joint.enable();
                else err = joint.disable();
                if (err != Error::None) return err;
            }
            return Error::None;
        }, [ctx](Error err){
            if (err != Error::None)
                ctx.respond(ResponseStatus::InvalidParameters);
            else ctx.respond(ResponseStatus::Ok);
        });
    }

    static void GetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Error::None || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<uint8_t>([legId]() {
            uint8_t enabledFlag = 0;
            for (int i = 0; i < (int) ::Leg::JointId::Count; i++)
            {
                Joint& joint = Robot::GetInstance().getBody().getLeg((::Leg::Id)legId).getJoint((::Leg::JointId)i);
                if (joint.isEnabled())
                {
                    enabledFlag |= (1 << i);
                }
            }
            return enabledFlag;
        }, [ctx](uint8_t enabledFlag){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabledFlag, sizeof(enabledFlag));
        });
    }

    static void SetPosition(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Error::None || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float x_m, y_m, z_m;
        if (reader.read(x_m) != Error::None || reader.read(y_m) != Error::None || reader.read(z_m) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        bool clearOverrides;
        if (reader.read(clearOverrides) != Error::None)
        {
            clearOverrides = true; // clear overrides by default
        }

        if (clearOverrides)
        {
            for (int i = 0; i < (int) ::Leg::JointId::Count; i++)
            {
                Joint::Id joint_id = static_cast<Joint::Id>(legId * (int) ::Leg::JointId::Count + i);
                Robot::GetInstance().getDecisionLoop().askJointAngle(joint_id, 0, IPC::OverrideMode::None);
            }
        }

        Robot::GetInstance().getDecisionLoop().askLegPosition((::Leg::Id) legId, x_m, y_m, z_m);
        ctx.respond(ResponseStatus::Ok);
    }

    static void GetTargetPosition(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Error::None || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        const Vec3f& pos = Robot::GetInstance().getDecisionLoop().getControlIntent().leg_overrides[(int) legId].value_pos;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &pos, sizeof(pos));
    }

    static void GetGrounded(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        uint8_t legId;
        if (reader.read(legId) != Error::None || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        bool grounded = Robot::GetInstance().getBody().getLeg((::Leg::Id) legId).isGrounded();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &grounded, sizeof(grounded));
    }


    static ActionCallback actions[] = {
        SetEnabled,                // 0x00
        GetEnabled,                // 0x01
        SetPosition,               // 0x02
        GetTargetPosition,         // 0x03
        GetGrounded,               // 0x04
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}