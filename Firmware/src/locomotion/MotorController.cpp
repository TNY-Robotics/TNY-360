#include "locomotion/MotorController.hpp"
#include "locomotion/MotorCalibration.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"
#include <cstdio>

constexpr const char* TAG = "MotorController";

MotorController::MotorController() {}

MotorController::MotorController(uint8_t joint_id, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel)
    : joint_id(joint_id), motor_channel(motor_channel), analog_channel(analog_channel)
{
}

Status MotorController::init()
{
    if (Status err = MotorDriver::Init(); err != Status::Ok)
    {
        return err;
    }
    if (Status err = AnalogDriver::Init(); err != Status::Ok)
    {
        return err;
    }

    // Disable motor for safety
    if (Status err = disable(); err != Status::Ok)
    {
        return err;
    }
    
    return Status::Ok;
}

Status MotorController::deinit()
{
    // Disable motor for safety
    if (Status err = disable(); err != Status::Ok)
    {
        return err;
    }

    // NOTE : We don't deinit the MotorDriver or AnalogDriver here,
    //        as they may be used by other MotorController instances.

    return Status::Ok;
}

Status MotorController::enable()
{
    if (enabled) return Status::Ok; // already enabled
    
    enabled = true;
    __set_target_position(); // Send target position directly
    return Status::Ok;
}

Status MotorController::disable()
{
    // NOTE : No check if already disabled, just in case the motor has been enabled by another source

    enabled = false;
    __set_target_position(); // effective right now
    return Status::Ok;
}

bool MotorController::isEnabled() const
{
    return enabled;
}

Status MotorController::setTargetPosition(float position)
{
    // clamp position between 0.0 and 1.0 (don't try to go beyond mechanical stops)
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;

    // set target position
    target = position;
    if (Status err = __set_target_position(); err != Status::Ok)
        return err;

    return Status::Ok;
}

float MotorController::getTargetPosition() const
{
    return target;
}

float MotorController::getCurrentPosition() const
{
    AnalogDriver::Value voltage = 0;
    if (Status err = AnalogDriver::GetVoltage(analog_channel, voltage); err != Status::Ok)
    {
        return -1.0f; // error
    }

    const MotorCalibration::CalibrationData& calib_data = MotorCalibration::GetJointCalibrationData((Joint::Id) joint_id);
    return (voltage - calib_data.fb_min) / (calib_data.fb_max - calib_data.fb_min);
}

Status MotorController::__set_target_position()
{
    MotorDriver::Value dc_value = 0;
    if (enabled && !MotorCalibration::GetCalibrationState().running)
    {
        const MotorCalibration::CalibrationData& calib_data = MotorCalibration::GetJointCalibrationData((Joint::Id) joint_id);
        dc_value = static_cast<MotorDriver::Value>(calib_data.dc_min + target * (calib_data.dc_max - calib_data.dc_min));
    }

    if (Status err = MotorDriver::SetDutyCycle(motor_channel, dc_value); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}