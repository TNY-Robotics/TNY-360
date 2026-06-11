#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Motor
{
    constexpr uint8_t MODULE_ID = 0x06;

    static void SetDutyCycle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float duty_cycle;
        if (reader.read(duty_cycle) != Error::None)
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
        
        if (MotorDriver::SetDutyCycle(motor.getMotorChannel(), duty_cycle) != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        if (MotorDriver::SendData() != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }

        ctx.respond(ResponseStatus::Ok);
    }

    static void GetDutyCycle(const RequestContext& ctx, const uint8_t* payload)
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

        float duty_cycle;
        if (MotorDriver::GetDutyCycle(motor.getMotorChannel(), duty_cycle) != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }

        ctx.respond(ResponseStatus::Ok, (uint8_t*) &duty_cycle, sizeof(duty_cycle));
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

        ctx.respond(ResponseStatus::Ok, (uint8_t*) &motor.getCalibrationData(), sizeof(::MotorController::CalibrationData));
    }

    static void SetCalibrationData(const RequestContext& ctx, const uint8_t* payload)
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
        
        ::MotorController::CalibrationData calib_data;
        if (reader.read(calib_data.dc_min) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.dc_max) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.dc_deadband) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_min) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_max) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_noise) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_latency_ms) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_inverted) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.max_speed) != Error::None) { ctx.respond(ResponseStatus::InvalidParameters); return; }

        if (Error err = motor.setCalibrationData(calib_data); err != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    static void DeleteCalibrationData(const RequestContext& ctx, const uint8_t* payload)
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
        if (Error err = motor.deleteCalibrationData(); err != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
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
        SetDutyCycle,           // 0x00
        GetDutyCycle,           // 0x01
        GetCalibrationState,       // 0x02
        GetCalibrationData,        // 0x03
        SetCalibrationData,        // 0x04
        DeleteCalibrationData,     // 0x05
        StartCalibration,          // 0x06
        StopCalibration,           // 0x07
        GetCalibrationProgress,    // 0x08
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}