#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/Log.hpp"
#include "common/RPC.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Body
{
    constexpr uint8_t MODULE_ID = 0x03;

    static void SetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        uint16_t enabledFlag;
        if (reader.read(enabledFlag) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Error>([enabledFlag](){
            Error err;
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                bool isEnabled = (enabledFlag & (1 << i)) != 0;
                Joint* joint = Joint::GetJoint((Joint::Id) i);
                if (joint == nullptr) continue;
                if (isEnabled) err = joint->enable();
                else err = joint->disable();
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
        RPC::ExecuteThreadSafe<uint16_t>([]() {
            uint16_t enabledFlag = 0;
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                Joint* joint = Joint::GetJoint((Joint::Id) i);
                if (joint != nullptr && joint->isEnabled())
                {
                    enabledFlag |= (1 << i);
                }
            }
            return enabledFlag;
        }, [ctx](uint16_t enabledFlag){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabledFlag, sizeof(enabledFlag));
        });
    }

    static void SetVelocity(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        float x_ms, y_ms, z_rads;
        if (reader.read(x_ms) != Error::None || reader.read(y_ms) != Error::None || reader.read(z_rads) != Error::None)
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
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                Robot::GetInstance().getDecisionLoop().askJointAngle((Joint::Id) i, 0, IPC::OverrideMode::None);
            }
        }
        Robot::GetInstance().getDecisionLoop().askBodyVelocity(x_ms, y_ms, z_rads);
        ctx.respond(ResponseStatus::Ok);
    }

    static void GetTargetVelocity(const RequestContext& ctx, const uint8_t* payload)
    {
        Vec3f vel = Robot::GetInstance().getDecisionLoop().getControlIntent().body_vel;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &vel, sizeof(vel));
    }

    static void SetPosture(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        float x_pos, y_pos, z_pos;
        if (reader.read(x_pos) != Error::None || reader.read(y_pos) != Error::None || reader.read(z_pos) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        float x_rot, y_rot, z_rot;
        if (reader.read(x_rot) != Error::None || reader.read(y_rot) != Error::None || reader.read(z_rot) != Error::None)
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
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                Robot::GetInstance().getDecisionLoop().askJointAngle((Joint::Id) i, 0, IPC::OverrideMode::None);
            }
        }
        Robot::GetInstance().getDecisionLoop().askBodyPosition(x_pos, y_pos, z_pos);
        Robot::GetInstance().getDecisionLoop().askBodyRotation(x_rot, y_rot, z_rot);
        ctx.respond(ResponseStatus::Ok);
    }

    static void GetPosture(const RequestContext& ctx, const uint8_t* payload)
    {
        const IPC::ControlIntent& intent = Robot::GetInstance().getDecisionLoop().getControlIntent();

        struct Posture {
            Vec3f pos;
            Vec3f rot;
        } posture;
        posture.pos = intent.body_pos;
        posture.rot = intent.body_rot;

        ctx.respond(ResponseStatus::Ok, (uint8_t*) &posture, sizeof(posture));
    }


    static ActionCallback actions[] = {
        SetEnabled,                // 0x00
        GetEnabled,                // 0x01
        SetVelocity,               // 0x02
        GetTargetVelocity,         // 0x03
        SetPosture,                // 0x04
        GetPosture,                // 0x05
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}