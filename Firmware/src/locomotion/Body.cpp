#include "locomotion/Body.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

Body::Body()
    : leg_fl(
        Joint(MotorController( 7,  9), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(MotorController( 5, 11), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(MotorController( 6, 10), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), false),  // Knee Pitch
        true
    ),
    leg_fr(
        Joint(MotorController( 4, 13), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(MotorController( 2, 15), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(MotorController( 3, 14), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), true ),  // Knee Pitch
        false
    ),
    leg_bl(
        Joint(MotorController(10,  5), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(MotorController( 8,  7), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(MotorController( 9,  6), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), false),  // Knee Pitch
        true
    ),
    leg_br(
        Joint(MotorController(13,  1), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(MotorController(11,  3), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(MotorController(12,  2), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), true ),  // Knee Pitch
        false
    ),
    ear_l(MotorController(1, 0), DEG_TO_RAD(-45.0f), DEG_TO_RAD(45.0f), false),
    ear_r(MotorController(0, 0), DEG_TO_RAD(-45.0f), DEG_TO_RAD(45.0f), false)
{
}

Error Body::init()
{
    Error err;

    if ((err = leg_fl.init()) != Error::None)
    {
        return err;
    }
    if ((err = leg_fr.init()) != Error::None)
    {
        return err;
    }
    if ((err = leg_bl.init()) != Error::None)
    {
        return err;
    }
    if ((err = leg_br.init()) != Error::None)
    {
        return err;
    }
    
    if ((err = ear_l.init()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.init()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Body::deinit()
{
    Error err;

    if ((err = leg_fl.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = leg_fr.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = leg_bl.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = leg_br.deinit()) != Error::None)
    {
        return err;
    }
    
    if ((err = ear_l.deinit()) != Error::None)
    {
        return err;
    }
    if ((err = ear_r.deinit()) != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error Body::update()
{
    Error err;

    if ((err = leg_fl.update()) != Error::None)
    {
        return err;
    }
    if ((err = leg_fr.update()) != Error::None)
    {
        return err;
    }
    if ((err = leg_bl.update()) != Error::None)
    {
        return err;
    }
    if ((err = leg_br.update()) != Error::None)
    {
        return err;
    }
    
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

    if ((err = leg_fl.enable()) != Error::None)
    {
        return err;
    }
    if ((err = leg_fr.enable()) != Error::None)
    {
        return err;
    }
    if ((err = leg_bl.enable()) != Error::None)
    {
        return err;
    }
    if ((err = leg_br.enable()) != Error::None)
    {
        return err;
    }
    
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

    if ((err = leg_fl.disable()) != Error::None)
    {
        return err;
    }
    if ((err = leg_fr.disable()) != Error::None)
    {
        return err;
    }
    if ((err = leg_bl.disable()) != Error::None)
    {
        return err;
    }
    if ((err = leg_br.disable()) != Error::None)
    {
        return err;
    }
    
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
    Leg* legs[4] = {
        &leg_fl,
        &leg_fr,
        &leg_bl,
        &leg_br
    };
    Vec3f local_hip_positions_mm[4] = {
        Vec3f( HIP_POS_X_MM,  HIP_POS_Y_MM, 0.f), // Front Left
        Vec3f( HIP_POS_X_MM, -HIP_POS_Y_MM, 0.f), // Front Right
        Vec3f(-HIP_POS_X_MM,  HIP_POS_Y_MM, 0.f), // Back Left
        Vec3f(-HIP_POS_X_MM, -HIP_POS_Y_MM, 0.f)  // Back Right
    };
    Vec3f global_feet_positions_mm[4] = {
        Vec3f( HIP_POS_X_MM,  80, 0.f), // Front Left
        Vec3f( HIP_POS_X_MM, -80, 0.f), // Front Right
        Vec3f(-HIP_POS_X_MM,  80, 0.f), // Back Left
        Vec3f(-HIP_POS_X_MM, -80, 0.f)  // Back Right
    };

    Vec3f targets_for_legs[4];
    float max_time_s = 0.f;

    for (int i = 0; i < 4; i++)
    {
        Vec3f foot_pos_in_body_frame = posture.worldToLocal(global_feet_positions_mm[i]);
        targets_for_legs[i] = foot_pos_in_body_frame - local_hip_positions_mm[i];
        float time_estimate = legs[i]->getTimeEstimate(targets_for_legs[i]);
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

    for (int i = 0; i < 4; i++)
    {
        Error err = legs[i]->setTarget_Timed(targets_for_legs[i], max_time_s);
        if (err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to set target for leg %d", i);
            return err;
        }
    }

    return Error::None;
}