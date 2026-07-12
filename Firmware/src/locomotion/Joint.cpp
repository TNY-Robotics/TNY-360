#include "locomotion/Joint.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

Joint* Joint::joints[JOINT_COUNT] = { nullptr }; // Static array to hold Joint instances
float Joint::joint_velocity_clamp_rad_s = Joint::MAX_VELOCITY_RAD_S; // Initialize static max velocity variable

Joint* Joint::GetJoint(Joint::Id id)
{
    if (id >= Joint::Id::Count)
    {
        return nullptr;
    }
    return joints[(int)id];
}

Status Joint::ClampVelocity(float max_velocity_rad_s)
{
    if (max_velocity_rad_s < 0.f || max_velocity_rad_s > MAX_VELOCITY_RAD_S)
    {
        LOG_ERROR(TAG, "Requested max joint velocity %.2f rad/s is out of bounds (0 - %.2f rad/s)", max_velocity_rad_s, MAX_VELOCITY_RAD_S);
        return Status::InvalidParameters;
    }
    if (max_velocity_rad_s == 0.f) // if zero, disable clamp
    {
        joint_velocity_clamp_rad_s = MAX_VELOCITY_RAD_S;
        return Status::Ok;
    }

    joint_velocity_clamp_rad_s = max_velocity_rad_s;
    return Status::Ok;
}

Joint::Joint() {}

Joint::Joint(Joint::Id id, MotorController motor_controller, float min_angle_rad, float max_angle_rad, bool inverted, bool has_feedback)
    : id(id), motor_controller(motor_controller), min_angle_rad(min_angle_rad), max_angle_rad(max_angle_rad),
      inverted(inverted), velocity_rad_s(MAX_VELOCITY_RAD_S)
{
}

Status Joint::init()
{
    LOG_SCOPE(TAG, "Joint::init [id=%d]", id);

    // Register this joint instance
    // NOTE : Don't move this in the constructor, as we can have copy/move operations in constructors arguments (yes, that will be horrible to debug)
    joints[(int)id] = this;

    if (Status err = motor_controller.init(); err != Status::Ok)
    {
        return err;
    }

    if (motor_controller.getMotorAttributes().has_feedback)
    {
        // Initialize model angle with current feedback
        if (Status err = get_motorcontroller_position(model_angle_rad); err != Status::Ok)
        {
            return err;
        }
        // Clamp the model angle within limits (just in case)
        if(model_angle_rad > max_angle_rad) model_angle_rad = max_angle_rad;
        if(model_angle_rad < min_angle_rad) model_angle_rad = min_angle_rad;

        // Initialize feedback and estimate angles
        feedback_angle_rad = model_angle_rad;
        estimate_angle_rad = model_angle_rad;

        // Set the target angle to the current position
        target_angle_rad = model_angle_rad;

        // Initialize Kalman filter
        constexpr float SENSOR_NOISE_VARIANCE = 1.f;
        constexpr float PROCESS_NOISE_VARIANCE = 0.1f;
        kalman_filter.Init(SENSOR_NOISE_VARIANCE, PROCESS_NOISE_VARIANCE, model_angle_rad);
    }
    else // no feedback, set to half the servo course
    {
        model_angle_rad = (min_angle_rad + max_angle_rad) / 2;
        feedback_angle_rad = model_angle_rad;
        estimate_angle_rad = model_angle_rad;
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
    if (motor_controller.getMotorAttributes().has_feedback)
    {
        // Get the raw feedback from the motor controller
        if (Status err = get_motorcontroller_position(feedback_angle_rad); err != Status::Ok)
        {
            return err;
        }

        // Update the Kalman filter
        estimate_angle_rad = kalman_filter.Update(feedback_angle_rad);
    }
    else // no feedback, use model as feedback and estimation
    {
        feedback_angle_rad = model_angle_rad;
        estimate_angle_rad = model_angle_rad;
    }

    return Status::Ok;
}

Status Joint::applyCommand(float joint_angle_rad, float dt)
{
    if (joint_angle_rad < min_angle_rad || joint_angle_rad > max_angle_rad)
    {
        LOG_ERROR(TAG, "JOINT %d - Requested target angle %.2f rad is out of bounds (%.2f - %.2f rad)", id, joint_angle_rad, min_angle_rad, max_angle_rad);
        return Status::InvalidParameters;
    }
    target_angle_rad = joint_angle_rad;

    if (motor_controller.getState() == MotorController::State::DISABLED)
    {
        // If the motor is disabled, don't do anything
        return Status::Ok;
    }

    float clamped_velocity_rad_s = std::min(velocity_rad_s, joint_velocity_clamp_rad_s);

    // Update model angle based on velocity and target
    float last_model_angle_rad = model_angle_rad;
    if (joint_angle_rad > model_angle_rad)
    {
        model_angle_rad += clamped_velocity_rad_s * CONTROL_LOOP_DT_S;
        if (model_angle_rad > joint_angle_rad)
        {
            model_angle_rad = joint_angle_rad;
        }
    }
    else if (joint_angle_rad < model_angle_rad)
    {
        model_angle_rad -= clamped_velocity_rad_s * CONTROL_LOOP_DT_S;
        if (model_angle_rad < joint_angle_rad)
        {
            model_angle_rad = joint_angle_rad;
        }
    }

    // If no feedback, set feedback and estimate angles to model
    if (!motor_controller.getMotorAttributes().has_feedback)
    {
        feedback_angle_rad = model_angle_rad;
        estimate_angle_rad = model_angle_rad;
    }

    // If feedback, indicate command to kalman filter
    if (motor_controller.getMotorAttributes().has_feedback)
    {
        kalman_filter.Predict(model_angle_rad - last_model_angle_rad);
    }

    // Move the motor at the desired position
    // NOTE : We could maybe improve this by moving the motor a bit ahead (to catch up with the control loop delay)
    if (Status err = send_motorcontroller_position(model_angle_rad); err != Status::Ok)
    {
        return err;
    }
    
    return Status::Ok;
}

Status Joint::enable()
{
    if (motor_controller.getMotorAttributes().has_feedback)
    {
        // because the motor was probably disabled before,
        // model_angle may have drifted from the actual position.
        // Thus, to avoid sudden jumps, we reset the model and kalman filter to the current position.
        if (Status err = get_motorcontroller_position(feedback_angle_rad); err != Status::Ok)
        {
            return err;
        }
        model_angle_rad = feedback_angle_rad;
        estimate_angle_rad = feedback_angle_rad;
        target_angle_rad = feedback_angle_rad;
        kalman_filter.ResetState(feedback_angle_rad);
        send_motorcontroller_position(feedback_angle_rad);

        // Now that the model is synced, enable the motor
        return motor_controller.enable();
    }
    else
    {
        // no feedback, nothing to sync, just enable the motor
        return motor_controller.enable();
    }
}

Status Joint::disable()
{
    return motor_controller.disable();
}

bool Joint::isEnabled() const
{
    return motor_controller.getState() == MotorController::State::ENABLED;
}

Status Joint::setVelocity(float velocity_rad_s)
{
    // clamp velocity to allowed range
    if (velocity_rad_s < 0.f || velocity_rad_s > MAX_VELOCITY_RAD_S)
    {
        // NOTE : Not logging error because we are in the control loop (200Hz) and we don't want to slow it down with logging
        // LOG_ERROR(TAG, "JOINT %d - Requested velocity %.2f rad/s is out of bounds (0 - %.2f rad/s)", id, velocity_rad_s, MAX_VELOCITY_RAD_S);
        return Status::InvalidParameters;
    }
    
    this->velocity_rad_s = velocity_rad_s;
    return Status::Ok;
}

Status Joint::getVelocity(float &result) const
{
    result = std::min(velocity_rad_s, joint_velocity_clamp_rad_s);
    return Status::Ok;
}

Status Joint::getTarget(float &result) const
{
    result = target_angle_rad;
    return Status::Ok;
}

Status Joint::getPosition(float &result) const
{
    result = estimate_angle_rad;
    return Status::Ok;
}

Status Joint::getFeedback(float &result) const
{
    result = feedback_angle_rad;
    return Status::Ok;
}

Status Joint::getPrediction(float &result) const
{
    result = model_angle_rad;
    return Status::Ok;
}

Status Joint::getUncertainty(float &result) const
{
    result = kalman_filter.GetUncertainty();
    return Status::Ok;
}


Status Joint::send_motorcontroller_position(const float& position)
{
    float position_ratio = (position - min_angle_rad) / (max_angle_rad - min_angle_rad);
    if (inverted)
    {
        position_ratio = 1.0f - position_ratio;
    }
    return motor_controller.setTargetPosition(position_ratio);
}

Status Joint::get_motorcontroller_position(float &result) const
{
    // if no feedback, just use the internal model
    if (!motor_controller.getMotorAttributes().has_feedback)
    {
        result = model_angle_rad;
        return Status::Ok;
    }

    float position_ratio;
    Status err = motor_controller.getCurrentPosition(position_ratio);
    if (err != Status::Ok)
    {
        return err;
    }
    if (inverted)
    {
        position_ratio = 1.0f - position_ratio;
    }
    result = min_angle_rad + position_ratio * (max_angle_rad - min_angle_rad);
    return Status::Ok;
}