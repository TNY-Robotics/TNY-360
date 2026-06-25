#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Motor
{
    constexpr uint8_t MODULE_ID = 0x06;

    /** <API_REF>
     * @module motor 0x06
     * @action setDutyCycle 0x00
     * @desc Sets the duty cycle of a motor.
     * @arg joint_id uint8 ID of the joint to set the duty cycle for.
     * @arg duty_cycle float32 Duty cycle to set in ms (between 0.0 and 5.0).
     * @impl done
     */
    static void SetDutyCycle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float duty_cycle;
        if (reader.read(duty_cycle) != Status::Ok)
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
        
        if (MotorDriver::SetDutyCycle(motor.getMotorChannel(), duty_cycle) != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        if (MotorDriver::SendData() != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }

        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module motor 0x06
     * @action getDutyCycle 0x01
     * @desc Gets the current duty cycle of a motor.
     * @arg joint_id uint8 ID of the joint to get the duty cycle for.
     * @result duty_cycle float32 Current duty cycle in ms (between 0.0 and 5.0).
     * @impl done
     */
    static void GetDutyCycle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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
        if (MotorDriver::GetDutyCycle(motor.getMotorChannel(), duty_cycle) != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }

        ctx.respond(ResponseStatus::Ok, (uint8_t*) &duty_cycle, sizeof(duty_cycle));
    }

    /** <API_REF>
     * @module motor 0x06
     * @action getCalibrationState 0x02
     * @desc Gets the current calibration state of a motor.
     * @arg joint_id uint8 ID of the joint to get the calibration state for.
     * @result calib_state uint8 Current calibration state (0 = Not Calibrated, 1 = Calibrating, 2 = Calibrated).
     * @impl done
     */
    static void GetCalibrationState(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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

    /** <API_REF>
     * @module motor 0x06
     * @action getCalibrationData 0x03
     * @desc Gets the calibration data of a motor.
     * @arg joint_id uint8 ID of the joint to get the calibration data for.
     * @result calib_data CalibrationData Current calibration data.
     * @impl done
     */
    static void GetCalibrationData(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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

    /** <API_REF>
     * @module motor 0x06
     * @action setCalibrationData 0x04
     * @desc Sets the calibration data of a motor.
     * @arg joint_id uint8 ID of the joint to set the calibration data for.
     * @arg calib_data CalibrationData New calibration data.
     * @impl done
     */
    static void SetCalibrationData(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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
        if (reader.read(calib_data.dc_min) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.dc_max) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.dc_deadband) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_min) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_max) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_noise) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_latency_ms) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.feedback_inverted) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }
        if (reader.read(calib_data.max_speed) != Status::Ok) { ctx.respond(ResponseStatus::InvalidParameters); return; }

        if (Status err = motor.setCalibrationData(calib_data); err != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module motor 0x06
     * @action deleteCalibrationData 0x05
     * @desc Deletes the calibration data of a motor.
     * @arg joint_id uint8 ID of the joint to delete the calibration data for.
     * @impl done
     */
    static void DeleteCalibrationData(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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
        if (Status err = motor.deleteCalibrationData(); err != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module motor 0x06
     * @action startCalibration 0x06
     * @desc Starts the calibration process for a motor.
     * @arg joint_id uint8 ID of the joint to start the calibration for.
     * @impl done
     */
    static void StartCalibration(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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
        if (Status err = motor.startCalibration(); err != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module motor 0x06
     * @action stopCalibration 0x07
     * @desc Stops the calibration process for a motor.
     * @arg joint_id uint8 ID of the joint to stop the calibration for.
     * @impl done
     */
    static void StopCalibration(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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
        if (Status err = motor.stopCalibration(); err != Status::Ok)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module motor 0x06
     * @action getCalibrationProgress 0x08
     * @desc Gets the progress of the calibration process for a motor.
     * @arg joint_id uint8 ID of the joint to get the calibration progress for.
     * @result progress float32 Current calibration progress (0.0 to 1.0).
     * @impl done
     */
    static void GetCalibrationProgress(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t joint_id;
        if (reader.read(joint_id) != Status::Ok)
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