#include "locomotion/Joint.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

Joint* Joint::joints[JOINT_COUNT] = { nullptr }; // Static array to hold Joint instances

Joint* Joint::GetJoint(MotorDriver::Channel motor_channel)
{
    if (motor_channel >= JOINT_COUNT)
    {
        return nullptr;
    }
    return joints[motor_channel];
}

Joint::Joint() {}

Joint::Joint(MotorController motor_controller, float min_angle_rad, float max_angle_rad, bool inverted)
    : motor_controller(motor_controller), min_angle_rad(min_angle_rad), max_angle_rad(max_angle_rad),
      inverted(inverted), velocity_rad_s(MAX_VELOCITY_RAD_S)
{
}

Error Joint::init()
{
    // Register this joint instance
    MotorDriver::Channel channel = motor_controller.getMotorChannel();
    if (channel < JOINT_COUNT)
    {
        joints[channel] = this;
    }
    // NOTE : Don't move this in the constructor, as we can have copy/move operations in constructors arguments (yes, that will be horrible to debug)

    if (Error err = motor_controller.init(); err != Error::None)
    {
        return err;
    }

    // Initialize model angle with current feedback
    if (Error err = get_motorcontroller_position(model_angle_rad); err != Error::None)
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
    if (Error err = setTarget(model_angle_rad); err != Error::None)
    {
        return err;
    }

    // Disable the motor initially
    if (Error err = disable(); err != Error::None)
    {
        return err;
    }

    // Initialize Kalman filter
    constexpr float SENSOR_NOISE_VARIANCE = 1.f;
    constexpr float PROCESS_NOISE_VARIANCE = 0.1f;
    kalman_filter.Init(SENSOR_NOISE_VARIANCE, PROCESS_NOISE_VARIANCE, model_angle_rad);

    return Error::None;
}

Error Joint::deinit()
{
    return motor_controller.deinit();
}

Error Joint::update()
{
    // Update model angle based on velocity and target
    float last_model_angle_rad = model_angle_rad;
    if (target_angle_rad > model_angle_rad)
    {
        model_angle_rad += velocity_rad_s * CONTROL_LOOP_DT_S;
        if (model_angle_rad > target_angle_rad)
        {
            model_angle_rad = target_angle_rad;
        }
    }
    else if (target_angle_rad < model_angle_rad)
    {
        model_angle_rad -= velocity_rad_s * CONTROL_LOOP_DT_S;
        if (model_angle_rad < target_angle_rad)
        {
            model_angle_rad = target_angle_rad;
        }
    }

    // Get the raw feedback from the motor controller
    if (Error err = get_motorcontroller_position(feedback_angle_rad); err != Error::None)
    {
        return err;
    }

    // Update the Kalman filter
    kalman_filter.Predict(model_angle_rad - last_model_angle_rad);
    estimate_angle_rad = kalman_filter.Update(feedback_angle_rad);

    // Move the motor at the desired position
    // NOTE : We could maybe improve this by moving the motor a bit ahead (to catch up with the control loop delay)
    if (Error err = send_motorcontroller_position(model_angle_rad); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Joint::enable()
{
    // because the motor was probably disabled before,
    // model_angle may have drifted from the actual position.
    // Thus, to avoid sudden jumps, we reset the model and estimate to the current position.
    if (Error err = get_motorcontroller_position(feedback_angle_rad); err != Error::None)
    {
        return err;
    }
    model_angle_rad = feedback_angle_rad;
    estimate_angle_rad = feedback_angle_rad;
    target_angle_rad = feedback_angle_rad;
    kalman_filter.ResetState(feedback_angle_rad);

    // Also force the controller to go to the current position
    if (Error err = send_motorcontroller_position(model_angle_rad); err != Error::None)
    {
        return err;
    }

    // Now that the model is synced, enable the motor
    return motor_controller.enable();
}

Error Joint::disable()
{
    return motor_controller.disable();
}

bool Joint::isEnabled() const
{
    return motor_controller.getState() == MotorController::State::ENABLED;
}

Error Joint::setVelocity(float velocity_rad_s)
{
    // clamp velocity to allowed range
    if (velocity_rad_s < 0.f || velocity_rad_s > MAX_VELOCITY_RAD_S)
    {
        Log::Add(Log::Level::Error, TAG, "JOINT %d - Requested velocity %.2f rad/s is out of bounds (0 - %.2f rad/s)", motor_controller.getMotorChannel(), velocity_rad_s, MAX_VELOCITY_RAD_S);
        return Error::InvalidParameters;
    }
    
    this->velocity_rad_s = velocity_rad_s;
    return Error::None;
}

Error Joint::getVelocity(float &result) const
{
    result = velocity_rad_s;
    return Error::None;
}

Error Joint::setTarget(float angle_rad)
{
    if (angle_rad < min_angle_rad || angle_rad > max_angle_rad)
    {
        Log::Add(Log::Level::Error, TAG, "JOINT %d - Requested target angle %.2f rad is out of bounds (%.2f - %.2f rad)", motor_controller.getMotorChannel(), angle_rad, min_angle_rad, max_angle_rad);
        return Error::InvalidParameters;
    }

    setVelocity(MAX_VELOCITY_RAD_S); // reset to max velocity for normal setTarget calls

    target_angle_rad = angle_rad;
    return Error::None;
}

Error Joint::getTarget(float &result) const
{
    result = target_angle_rad;
    return Error::None;
}

Error Joint::setTarget_Timed(float angle_rad, float time_s)
{
    if (angle_rad < min_angle_rad || angle_rad > max_angle_rad)
    {
        Log::Add(Log::Level::Error, TAG, "JOINT %d - Requested target angle %.2f rad is out of bounds (%.2f - %.2f rad)", motor_controller.getMotorChannel(), angle_rad, min_angle_rad, max_angle_rad);
        return Error::InvalidParameters;
    }

    // calculate and set required velocity
    float distance_rad = std::abs(angle_rad - model_angle_rad);
    float required_velocity = distance_rad / time_s;

    if (Error err = setVelocity(required_velocity); err != Error::None)
    {
        return err;
    }

    // set the target angle
    target_angle_rad = angle_rad;
    return Error::None;
}

Error Joint::getPosition(float &result) const
{
    result = estimate_angle_rad;
    return Error::None;
}

Error Joint::getFeedback(float &result) const
{
    result = feedback_angle_rad;
    return Error::None;
}

Error Joint::getPrediction(float &result) const
{
    result = model_angle_rad;
    return Error::None;
}

Error Joint::getUncertainty(float &result) const
{
    result = kalman_filter.GetUncertainty();
    return Error::None;
}

float Joint::getTimeEstimate(float target_angle_rad) const
{
    float distance_rad = std::abs(target_angle_rad - model_angle_rad);
    float time_s = distance_rad / MAX_VELOCITY_RAD_S;
    return time_s;
}


Error Joint::send_motorcontroller_position(const float& position)
{
    float position_ratio = (position - min_angle_rad) / (max_angle_rad - min_angle_rad);
    if (inverted)
    {
        position_ratio = 1.0f - position_ratio;
    }
    return motor_controller.setTargetPosition(position_ratio);
}

Error Joint::get_motorcontroller_position(float &result) const
{
    float position_ratio;
    Error err = motor_controller.getCurrentPosition(position_ratio);
    if (err != Error::None)
    {
        return err;
    }
    if (inverted)
    {
        position_ratio = 1.0f - position_ratio;
    }
    result = min_angle_rad + position_ratio * (max_angle_rad - min_angle_rad);
    return Error::None;
}