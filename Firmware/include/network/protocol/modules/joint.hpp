#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "locomotion/Joint.hpp"
#include "Robot.hpp"
#include "common/Log.hpp"
#include "common/RPC.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Joint
{
    constexpr uint8_t MODULE_ID = 0x05;

    static void SetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        bool enabled;
        if (reader.read(enabled) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Error>([joint, enabled](){
            Error err;
            if (enabled) err = joint->enable();
            else err = joint->disable();
            return err;
        }, [ctx](Error err){
            if (err != Error::None)
                ctx.respond(ResponseStatus::InvalidParameters);
            else ctx.respond(ResponseStatus::Ok);
        });
    }

    static void GetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<bool>([joint](){
            return joint->isEnabled();
        }, [ctx](bool enabled){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabled, sizeof(enabled));
        });
    }

    static void SetAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float angle;
        if (reader.read(angle) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        Robot::GetInstance().getDecisionLoop().askJointAngle((::Joint::Id) jointId, angle);
        ctx.respond(ResponseStatus::Ok);
    }

    static void GetTargetAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        
        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float targetAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].target_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &targetAngle, sizeof(targetAngle));
    }

    static void GetFeedbackAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float feedbackAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].feedback_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &feedbackAngle, sizeof(feedbackAngle));
    }

    static void GetModelAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float modelAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].model_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &modelAngle, sizeof(modelAngle));
    }

    static void GetEstimatedAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float estimatedAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].estimated_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &estimatedAngle, sizeof(estimatedAngle));
    }

    static void SetJointAngles(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        float angles[14];
        for (int i = 0; i < 14; i++)
        {
            if (reader.read(angles[i]) != Error::None)
            {
                ctx.respond(ResponseStatus::InvalidParameters);
                return;
            }
        }

        for (int i = 0; i < 14; i++)
        {
            if (Robot::GetInstance().getDecisionLoop().askJointAngle((::Joint::Id) i, angles[i]) != Error::None)
            {
                ctx.respond(ResponseStatus::InvalidParameters);
                return;
            }
        }
        ctx.respond(ResponseStatus::Ok);
    }

    static void GetJointAngles(const RequestContext& ctx, const uint8_t* payload)
    {
        float angles[14];
        for (int i = 0; i < 14; i++)
        {
            angles[i] = Robot::GetInstance().getDecisionLoop().getRobotState().joints[i].feedback_angle_rad;
        }
        ctx.respond(ResponseStatus::Ok, (uint8_t*) angles, sizeof(angles));
    }


    static ActionCallback actions[] = {
        SetEnabled,                // 0x00
        GetEnabled,                // 0x01
        SetAngle,                  // 0x02
        GetTargetAngle,            // 0x03
        GetFeedbackAngle,          // 0x04
        GetModelAngle,             // 0x05
        GetEstimatedAngle,         // 0x06
        SetJointAngles,            // 0x07
        GetJointAngles             // 0x08
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}