#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Motor
{
    constexpr uint8_t MODULE_ID = 0x06;

    static void SetPWMDutyCycle(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    static void GetPWMDutyCycle(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    static void GetCalibrationState(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) joint_id);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::MotorController& motor = joint->getMotorController();
        ::MotorController::CalibrationState calib_state = motor.getCalibrationState();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &calib_state, sizeof(calib_state));
    }

    static void GetCalibrationData(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    static void SetCalibrationData(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    static void StartCalibration(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) joint_id);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::MotorController& motor = joint->getMotorController();
        if (Error err = motor.startCalibration(); err != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    static void StopCalibration(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) joint_id);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::MotorController& motor = joint->getMotorController();
        if (Error err = motor.stopCalibration(); err != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    static void GetCalibrationProgress(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) joint_id);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::MotorController& motor = joint->getMotorController();
        float progress = motor.getCalibrationProgress();

        ctx.respond(ResponseStatus::Ok, (uint8_t*) &progress, sizeof(float));
    }


    static ActionCallback actions[] = {
        SetPWMDutyCycle,           // 0x00
        GetPWMDutyCycle,           // 0x01
        GetCalibrationState,       // 0x02
        GetCalibrationData,        // 0x03
        SetCalibrationData,        // 0x04
        StartCalibration,          // 0x05
        StopCalibration,           // 0x06
        GetCalibrationProgress,    // 0x07
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}