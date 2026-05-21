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

Error Body::init()
{
    LOG_SCOPE(TAG, "Body::Init");

    // Initialize the legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if (Error err = legs[i].init(); err != Error::None)
        {
            return err;
        }
    }

    // Initialize the ears
    if (Error err = ear_l.init(); err != Error::None)
    {
        return err;
    }
    if (Error err = ear_r.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the IMU
    if (Error err = imu.init(); err != Error::None)
    {
        return err;
    }

    // Initialize the PowerDriver
    if (Error err = PowerDriver::Init(); err != Error::None)
    {
        return err;
    }

    // Initialize the IPC layer (for inter-core infos)
    if (Error err = IPC::Init(); err != Error::None)
    {
        return err;
    }

    // Initialize the RPC layer (for inter-core calls)
    if (Error err = RPC::Init(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Body::deinit()
{
    // Deinitialize the RPC layer
    if (Error err = RPC::DeInit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the IPC layer (for inter-core infos)
    if (Error err = IPC::Deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the PowerDriver
    if (Error err = PowerDriver::Deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if (Error err = legs[i].deinit(); err != Error::None)
        {
            return err;
        }
    }
    
    // Deinitialize the ears
    if (Error err = ear_l.deinit(); err != Error::None)
    {
        return err;
    }
    if (Error err = ear_r.deinit(); err != Error::None)
    {
        return err;
    }

    // Deinitialize the IMU
    if (Error err = imu.deinit(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Body::estimateState(float dt)
{
    // update IMU
    if (Error err = imu.estimateState(dt); err != Error::None)
    {
        return err;
    }

    // update legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if (Error err = legs[i].estimateState(dt); err != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}

Error Body::applyCommand(BodyJointState jointState, float dt)
{
    Error err;

    // update legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if ((err = legs[i].applyCommand(jointState.leg_joints[i], dt)) != Error::None)
        {
            return err;
        }
    }
    
    // update ears
    if ((err = ear_l.applyCommand(jointState.ear_l_rad, dt)) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.applyCommand(jointState.ear_r_rad, dt)) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Body::enable()
{
    Error err;

    // enable legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if ((err = legs[i].enable()) != Error::None)
        {
            return err;
        }
    }
    
    // enable ears
    if ((err = ear_l.enable()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.enable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Body::disable()
{
    Error err;

    // disable legs
    for (size_t i = 0; i < static_cast<size_t>(Leg::Id::Count); i++)
    {
        if ((err = legs[i].disable()) != Error::None)
        {
            return err;
        }
    }
    
    // disable ears
    if ((err = ear_l.disable()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.disable()) != Error::None)
    {
        return err;
    }
    return Error::None;
}
