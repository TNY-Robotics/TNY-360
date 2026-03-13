#include "locomotion/Body.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "common/RPC.hpp"
#include "locomotion/IPC.hpp"

Body::Body()
{
    // Create the 4 legs
    legs[static_cast<size_t>(LegIndex::FRONT_LEFT)] = Leg(
        Joint(MotorController( 7,  9), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(MotorController( 5, 11), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(MotorController( 6, 10), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), false),  // Knee Pitch
        true
    );
    legs[static_cast<size_t>(LegIndex::FRONT_RIGHT)] = Leg(
        Joint(MotorController( 4, 13), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(MotorController( 2, 15), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(MotorController( 3, 14), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), true ),  // Knee Pitch
        false
    );
    legs[static_cast<size_t>(LegIndex::BACK_LEFT)] = Leg(
        Joint(MotorController(10,  5), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(MotorController( 8,  7), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(MotorController( 9,  6), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), false),  // Knee Pitch
        true
    );
    legs[static_cast<size_t>(LegIndex::BACK_RIGHT)] = Leg(
        Joint(MotorController(13,  1), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(MotorController(11,  3), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(MotorController(12,  2), DEG_TO_RAD(   0.0f), DEG_TO_RAD(150.0f), true ),  // Knee Pitch
        false
    );

    // Create the ears (default calib is for MG996R, using SG90 for ears)
    ear_l = Joint(MotorController(1, 0, MotorController::DEFAULT_CALIBRATION_SG90), DEG_TO_RAD(0.0f), DEG_TO_RAD(180.0f), true, false);
    ear_r = Joint(MotorController(0, 0, MotorController::DEFAULT_CALIBRATION_SG90), DEG_TO_RAD(0.0f), DEG_TO_RAD(180.0f), false, false);

    // Set the default posture and kinematic parameters
    local_hip_positions_mm[static_cast<size_t>(LegIndex::FRONT_LEFT)] = Vec3f( HIP_POS_X_MM,  HIP_POS_Y_MM, 0.f); // Front Left
    local_hip_positions_mm[static_cast<size_t>(LegIndex::FRONT_RIGHT)] = Vec3f( HIP_POS_X_MM, -HIP_POS_Y_MM, 0.f); // Front Right
    local_hip_positions_mm[static_cast<size_t>(LegIndex::BACK_LEFT)] = Vec3f(-HIP_POS_X_MM,  HIP_POS_Y_MM, 0.f); // Back Left
    local_hip_positions_mm[static_cast<size_t>(LegIndex::BACK_RIGHT)] = Vec3f(-HIP_POS_X_MM, -HIP_POS_Y_MM, 0.f); // Back Right

    // feet positions
    global_feet_positions_mm[static_cast<size_t>(LegIndex::FRONT_LEFT)] = Vec3f( DEFAULT_FEET_SPREAD_X_MM,  DEFAULT_FEET_SPREAD_Y_MM, 0.f); // Front Left
    global_feet_positions_mm[static_cast<size_t>(LegIndex::FRONT_RIGHT)] = Vec3f( DEFAULT_FEET_SPREAD_X_MM, -DEFAULT_FEET_SPREAD_Y_MM, 0.f); // Front Right
    global_feet_positions_mm[static_cast<size_t>(LegIndex::BACK_LEFT)] = Vec3f(-DEFAULT_FEET_SPREAD_X_MM,  DEFAULT_FEET_SPREAD_Y_MM, 0.f); // Back Left
    global_feet_positions_mm[static_cast<size_t>(LegIndex::BACK_RIGHT)] = Vec3f(-DEFAULT_FEET_SPREAD_X_MM, -DEFAULT_FEET_SPREAD_Y_MM, 0.f); // Back Right
}

Error Body::init()
{
    Error err;

    // Initialize the legs
    for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
    {
        if ((err = legs[i].init()) != Error::None)
        {
            return err;
        }
    }
    
    // Initialize the ears
    if ((err = ear_l.init()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.init()) != Error::None)
    {
        return err;
    }

    // Initialize the IMU
    if ((err = imu.init()) != Error::None)
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

    // Initialize the control loop
    if (Error err = control_loop.init(); err != Error::None)
    {
        return err;
    }

    return Error::None;
}

Error Body::deinit()
{
    // Deinitialize the control loop
    if (Error err = control_loop.init(); err != Error::None)
    {
        return err;
    }

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

    // Deinitialize the legs
    for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
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

Error Body::update()
{
    Error err;

    // Update the IMU controller
    if ((err = imu.update()) != Error::None)
    {
        return err;
    }

    // update legs
    for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
    {
        if ((err = legs[i].update()) != Error::None)
        {
            return err;
        }
    }
    
    // update ears
    if ((err = ear_l.update()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.update()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Body::enable()
{
    Error err;

    // enable legs
    for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
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
    for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
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

Error Body::setPosture(const Transformf& posture)
{
    this->posture = posture; 
    return apply_posture();
}

Error Body::getTargetPosture(Transformf& posture) const
{
    posture = this->posture;
    return Error::None;
}

Error Body::setFeetPosition(LegIndex leg_index, const Vec3f& position)
{
    if (leg_index >= LegIndex::COUNT)
    {
        return Error::InvalidParameters;
    }
    global_feet_positions_mm[static_cast<size_t>(leg_index)] = position;
    return apply_posture();
}

Error Body::getFeetPosition(LegIndex leg_index, Vec3f& position) const
{
    if (leg_index >= LegIndex::COUNT)
    {
        return Error::InvalidParameters;
    }
    position = global_feet_positions_mm[static_cast<size_t>(leg_index)];
    return Error::None;
}

Error Body::apply_posture()
{
    Vec3f targets_for_legs[4];
    float max_time_s = 0.f;

    for (int i = 0; i < static_cast<int>(LegIndex::COUNT); i++)
    {
        Vec3f foot_pos_in_body_frame = posture.worldToLocal(global_feet_positions_mm[i]);
        targets_for_legs[i] = foot_pos_in_body_frame - local_hip_positions_mm[i];
        float time_estimate = legs[i].getTimeEstimate(targets_for_legs[i]);
        if (time_estimate < 0.f)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to get time estimate for leg %d", i);
            return Error::Unknown;
        }
        if (time_estimate > max_time_s)
        {
            max_time_s = time_estimate;
        }
    }

    for (int i = 0; i < static_cast<int>(LegIndex::COUNT); i++)
    {
        Error err = legs[i].setTarget_Timed(targets_for_legs[i], max_time_s);
        if (err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to set target for leg %d", i);
            return err;
        }
    }

    return Error::None;
}