#include "locomotion/Joint.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "locomotion/MotorCalibration.hpp"
#include <algorithm> // For std::clamp

Joint* Joint::joints[static_cast<size_t>(Joint::Id::Count)] = { nullptr }; // Static array to hold Joint instances

Joint* Joint::GetJoint(Joint::Id id)
{
    if (id >= Joint::Id::Count) return nullptr;
    return joints[(int)id];
}

// FIXME : Use max velocity from calibration, or something else, but this is dirty
constexpr float MOTOR_VELOCITY = 6.28f; // rad/s

/// CLASS Joint

Joint::Joint(Joint::Id id, JointConfig config)
    : id(id), config(config)
{
    motor_controller = MotorController(
        (uint8_t) id,
        (MotorDriver::Channel)config.motor_driver_index,
        (AnalogDriver::Channel)config.analog_reader_index
    );
}

Status Joint::init()
{
    // Register this joint instance
    // NOTE : Don't move this in the constructor, as we can have copy/move operations in constructors arguments (yes, that will be horrible to debug)
    joints[(int)id] = this;

    // init/Load the calibration data
    if (Status err = MotorCalibration::Init(this->id); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to initialize MotorCalibration module with error [%s]", StatusToString(err));
        // TODO : Register error event
        return err;
    }

    // init the motor controller
    if (Status err = motor_controller.init(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to initialize motor controller for joint %d with error [%s]", id, StatusToString(err));
        // TODO : Register error event
        return err;
    }

    // Disable the motor initially
    if (Status err = disable(); err != Status::Ok)
    {
        return err;
    }
    
    return Status::Ok;
}

Status Joint::deinit()
{
    return motor_controller.deinit();
}

Status Joint::estimateState(float dt)
{
    float last_model_angle_rad = model_angle_rad; // for kalman filter prediction

    // Update internal motor model
    if (target_angle_rad > model_angle_rad)
    {
        model_angle_rad += MOTOR_VELOCITY * dt;
        if (model_angle_rad > target_angle_rad)
            model_angle_rad = target_angle_rad;
    }
    else if (target_angle_rad < model_angle_rad)
    {
        model_angle_rad -= MOTOR_VELOCITY * dt;
        if (model_angle_rad < target_angle_rad)
            model_angle_rad = target_angle_rad;
    }
    // TODO : This is a simple (kinematic) model,
    //        maybe we should detect acceleration in calibration and use a more complex model.

    // Update from "sensors"
    if (config.has_feedback)
    {
        // Get the raw feedback from the motor controller
        if (Status err = get_motor_position(feedback_angle_rad); err != Status::Ok)
        {
            return err;
        }

        // Update the Kalman filter
        kalman_filter.Predict(model_angle_rad - last_model_angle_rad);
        filtered_angle_rad = kalman_filter.Update(feedback_angle_rad);
    }
    else // no feedback, use model as feedback and estimation
    {
        feedback_angle_rad = model_angle_rad;
        filtered_angle_rad = model_angle_rad;
    }

    return Status::Ok;
}

Status Joint::applyCommand(float joint_angle_rad, float dt)
{
    // Clamp the target angle to the joint limits (should already be done in the MotorController, but just in case)
    if (joint_angle_rad < config.min_angle_rad || joint_angle_rad > config.max_angle_rad)
    {
        joint_angle_rad = std::clamp(joint_angle_rad, config.min_angle_rad, config.max_angle_rad);
        // TODO : Register an error event ?
    }
    target_angle_rad = joint_angle_rad; // save the target angle for future reference

    if (!motor_controller.isEnabled())
    {
        // If the motor is disabled, don't do anything
        return Status::Ok;
    }

    // Move the motor to the target position
    if (Status err = set_motor_position(joint_angle_rad); err != Status::Ok)
    {
        return err;
    }
    
    return Status::Ok;
}

Status Joint::enable()
{
    if (enabled) return Status::Ok; // already enabled

    if (config.has_feedback)
    {
        // because the motor was probably disabled before,
        // model_angle may have drifted from the actual position (if the motor has been manually moved)
        // Sync all the angles to the current feedback position, and reset the Kalman filter state
        if (Status err = get_motor_position(feedback_angle_rad); err != Status::Ok)
        {
            return err;
        }
        model_angle_rad = feedback_angle_rad;
        filtered_angle_rad = feedback_angle_rad;
        target_angle_rad = feedback_angle_rad;
        kalman_filter.ResetState(feedback_angle_rad);
        set_motor_position(feedback_angle_rad); // Actual motor sync (ask current position to avoid sudden jumps)

        // Enable the motor
        if (Status err = motor_controller.enable(); err != Status::Ok)
        {
            return err;
        }
    }

    // Motor has no feedback, we can't sync, we will enable it on the next applyCommand() call instead.
    enabled = true;
    return Status::Ok;
}

Status Joint::disable()
{
    enabled = false;
    return motor_controller.disable();
}

bool Joint::isEnabled() const
{
    return enabled;
}

float Joint::getTargetAngle() const
{
    return target_angle_rad;
}

float Joint::getCurrentAngle() const
{
    // FIXME : Should be the kalman filter (filtered_angle_rad), but i don't know if it works lmao
    return feedback_angle_rad;
}

Joint::Id Joint::getId() const
{
    return id;
}

const JointConfig& Joint::getConfig() const
{
    return config;
}

/// Internal

Status Joint::set_motor_position(const float& position)
{
    float position_ratio = (position - config.min_angle_rad) / (config.max_angle_rad - config.min_angle_rad);
    if (config.is_inverted) position_ratio = 1.0f - position_ratio;
    return motor_controller.setTargetPosition(position_ratio);
}

Status Joint::get_motor_position(float &result) const
{
    // if no feedback, just use the internal model
    if (!config.has_feedback)
    {
        result = model_angle_rad;
        return Status::Ok;
    }

    float position_ratio = motor_controller.getCurrentPosition();
    if (config.is_inverted) position_ratio = 1.0f - position_ratio;
    result = config.min_angle_rad + position_ratio * (config.max_angle_rad - config.min_angle_rad);
    return Status::Ok;
}