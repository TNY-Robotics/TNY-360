#include "locomotion/Leg.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"

Leg::Leg() {}

Leg::Leg(Joint hip_roll, Joint hip_pitch, Joint knee_pitch, AnalogDriver::Channel contact_channel, bool y_inverted)
    : contact_channel(contact_channel), y_inverted(y_inverted)
{
    joints[(int)JointId::HipRoll] = hip_roll;
    joints[(int)JointId::HipPitch] = hip_pitch;
    joints[(int)JointId::KneePitch] = knee_pitch;
    grounded = true; // by default on the ground
}

Status Leg::init()
{
    Status err;

    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if ((err = joints[i].init()) != Status::Ok)
        {
            return err;
        }
    }
    return Status::Ok;
}

Status Leg::deinit()
{
    Status err;

    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if ((err = joints[i].deinit()) != Status::Ok)
        {
            return err;
        }
    }
    return Status::Ok;
}

Status Leg::estimateState(float dt)
{
    Status err;

    // Check if grounded
    AnalogDriver::Value voltage;
    if (Status err = AnalogDriver::GetVoltage(this->contact_channel, voltage); err != Status::Ok)
    {
        return err;
    }
    grounded = voltage < LEG_GROUNDED_THRESHOLD_V;

    // pass the call to all motors
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if ((err = joints[i].estimateState(dt)) != Status::Ok)
        {
            return err;
        }
    }

    return Status::Ok;
}

Status Leg::applyCommand(LegJointState jointState, float dt)
{
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if (Status err = joints[i].applyCommand(jointState.joint_angles_rad[i], dt); err != Status::Ok)
        {
            return err;
        }
    }

    return Status::Ok;
}

Status Leg::enable()
{
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if (Status err = joints[i].enable(); err != Status::Ok)
        {
            return err;
        }
    }
    return Status::Ok;
}

Status Leg::disable()
{
    for (int i = 0; i < (int)JointId::Count; i++)
    {
        if (Status err = joints[i].disable(); err != Status::Ok)
        {
            return err;
        }
    }
    return Status::Ok;
}
