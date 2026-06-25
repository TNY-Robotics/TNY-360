#include "locomotion/Body.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "drivers/PowerDriver.hpp"
#include "common/RPC.hpp"
#include "locomotion/IPC.hpp"

Body::Body()
{
    // Create the 4 legs
    legs[static_cast<size_t>(Leg::Id::FrontLeft)] = Leg(
        Joint(Joint::Id::FrontLeftHipRoll, MotorController( 2,  0), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(Joint::Id::FrontLeftHipPitch, MotorController( 3,  1), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(Joint::Id::FrontLeftKneePitch, MotorController( 4,  2), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), false),  // Knee Pitch
        3, true
    );
    legs[static_cast<size_t>(Leg::Id::BackLeft)] = Leg(
        Joint(Joint::Id::BackLeftHipRoll, MotorController( 5,  4), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(Joint::Id::BackLeftHipPitch, MotorController( 6,  5), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(Joint::Id::BackLeftKneePitch, MotorController( 7,  6), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), false),  // Knee Pitch
        7, true
    );
    legs[static_cast<size_t>(Leg::Id::BackRight)] = Leg(
        Joint(Joint::Id::BackRightHipRoll, MotorController( 8,  8), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(Joint::Id::BackRightHipPitch, MotorController( 9,  9), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(Joint::Id::BackRightKneePitch, MotorController(10,  10), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), true ),  // Knee Pitch
        11, false
    );
    legs[static_cast<size_t>(Leg::Id::FrontRight)] = Leg(
        Joint(Joint::Id::FrontRightHipRoll, MotorController(11, 12), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(Joint::Id::FrontRightHipPitch, MotorController(12, 13), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(Joint::Id::FrontRightKneePitch, MotorController(13, 14), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), true ),  // Knee Pitch
        15, false
    );

    // Create the ears (default calib is for MG996R, using SG90 for ears)
    ear_r = Joint(Joint::Id::EarRight, MotorController(0, 0, MotorController::MOTOR_ATTR_SG90), DEG_TO_RAD(0.0f), DEG_TO_RAD(180.0f), false, false);
    ear_l = Joint(Joint::Id::EarLeft, MotorController(1, 0, MotorController::MOTOR_ATTR_SG90), DEG_TO_RAD(0.0f), DEG_TO_RAD(180.0f), true, false);
}

Status Body::init()
{
    LOG_SCOPE(TAG, "Body::Init");

    // Initialize the legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if (Status err = legs[i].init(); err != Status::Ok)
        {
            return err;
        }
    }

    // Initialize the ears
    if (Status err = ear_l.init(); err != Status::Ok)
    {
        return err;
    }
    if (Status err = ear_r.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the IMU
    if (Status err = imu.init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the PowerDriver
    if (Status err = PowerDriver::Init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the IPC layer (for inter-core infos)
    if (Status err = IPC::Init(); err != Status::Ok)
    {
        return err;
    }

    // Initialize the RPC layer (for inter-core calls)
    if (Status err = RPC::Init(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}

Status Body::deinit()
{
    // Deinitialize the RPC layer
    if (Status err = RPC::DeInit(); err != Status::Ok)
    {
        return err;
    }

    // Deinitialize the IPC layer (for inter-core infos)
    if (Status err = IPC::Deinit(); err != Status::Ok)
    {
        return err;
    }

    // Deinitialize the PowerDriver
    if (Status err = PowerDriver::Deinit(); err != Status::Ok)
    {
        return err;
    }

    // Deinitialize the legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if (Status err = legs[i].deinit(); err != Status::Ok)
        {
            return err;
        }
    }
    
    // Deinitialize the ears
    if (Status err = ear_l.deinit(); err != Status::Ok)
    {
        return err;
    }
    if (Status err = ear_r.deinit(); err != Status::Ok)
    {
        return err;
    }

    // Deinitialize the IMU
    if (Status err = imu.deinit(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}

Status Body::estimateState(float dt)
{
    // update IMU
    if (Status err = imu.estimateState(dt); err != Status::Ok)
    {
        return err;
    }

    // update legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if (Status err = legs[i].estimateState(dt); err != Status::Ok)
        {
            return err;
        }
    }
    return Status::Ok;
}

Status Body::applyCommand(BodyJointState jointState, float dt)
{
    Status err;

    // update legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if ((err = legs[i].applyCommand(jointState.leg_joints[i], dt)) != Status::Ok)
        {
            return err;
        }
    }
    
    // update ears
    if ((err = ear_l.applyCommand(jointState.ear_l_rad, dt)) != Status::Ok)
    {
        return err;
    }
    if ((err = ear_r.applyCommand(jointState.ear_r_rad, dt)) != Status::Ok)
    {
        return err;
    }
    return Status::Ok;
}

Status Body::enable()
{
    Status err;

    // enable legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if ((err = legs[i].enable()) != Status::Ok)
        {
            return err;
        }
    }
    
    // enable ears
    if ((err = ear_l.enable()) != Status::Ok)
    {
        return err;
    }
    if ((err = ear_r.enable()) != Status::Ok)
    {
        return err;
    }
    return Status::Ok;
}

Status Body::disable()
{
    Status err;

    // disable legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if ((err = legs[i].disable()) != Status::Ok)
        {
            return err;
        }
    }
    
    // disable ears
    if ((err = ear_l.disable()) != Status::Ok)
    {
        return err;
    }
    if ((err = ear_r.disable()) != Status::Ok)
    {
        return err;
    }
    return Status::Ok;
}
