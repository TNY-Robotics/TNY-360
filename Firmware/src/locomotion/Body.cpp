#include "locomotion/Body.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

Body::Body()
{
    // Create the 4 legs
    legs[static_cast<size_t>(LegIndex::FRONT_LEFT)] = Leg(
        Joint(MotorController( 7,  9), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(MotorController( 5, 11), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(MotorController( 6, 10), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), false),  // Knee Pitch
        true
    );
    legs[static_cast<size_t>(LegIndex::FRONT_RIGHT)] = Leg(
        Joint(MotorController( 4, 13), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(MotorController( 2, 15), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(MotorController( 3, 14), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), true ),  // Knee Pitch
        false
    );
    legs[static_cast<size_t>(LegIndex::BACK_LEFT)] = Leg(
        Joint(MotorController(10,  5), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Roll
        Joint(MotorController( 8,  7), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), true ),  // Hip Pitch
        Joint(MotorController( 9,  6), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), false),  // Knee Pitch
        true
    );
    legs[static_cast<size_t>(LegIndex::BACK_RIGHT)] = Leg(
        Joint(MotorController(13,  1), DEG_TO_RAD( -45.0f), DEG_TO_RAD( 45.0f), false),  // Hip Roll
        Joint(MotorController(11,  3), DEG_TO_RAD(-135.0f), DEG_TO_RAD( 45.0f), false),  // Hip Pitch
        Joint(MotorController(12,  2), DEG_TO_RAD(   0.0f), DEG_TO_RAD(135.0f), true ),  // Knee Pitch
        false
    );

    // Create the ears
    ear_l = Joint(MotorController(1, 0), DEG_TO_RAD(-45.0f), DEG_TO_RAD(45.0f), false);
    ear_r = Joint(MotorController(0, 0), DEG_TO_RAD(-45.0f), DEG_TO_RAD(45.0f), false);

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

    // init legs
    for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
    {
        if ((err = legs[i].init()) != Error::None)
        {
            return err;
        }
    }
    
    // init ears
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

    // deinit legs
    for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
    {
        if ((err = legs[i].deinit()) != Error::None)
        {
            return err;
        }
    }
    
    // deinit ears
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

Error Body::startCalibration()
{
    xTaskCreate([](void* param) {
        Body* body = static_cast<Body*>(param);
        Error err;

        // Disable all joints before calibration
        Log::Add(Log::Level::Info, Body::TAG, "Disabling all joints before calibration...");
        for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
        {
            Joint* joints[] = {
                &body->legs[i].getHipRoll(),
                &body->legs[i].getHipPitch(),
                &body->legs[i].getKneePitch()
            };
            int nb_joints = sizeof(joints) / sizeof(joints[0]);

            for (int j = 0; j < nb_joints; j++)
            {
                Joint* joint = joints[j];
                if ((err = joint->disable()) != Error::None)
                {
                    Log::Add(Log::Level::Error, Body::TAG, "Failed to disable a joint before calibration. Error: %d", static_cast<uint8_t>(err));
                    vTaskDelete(nullptr);
                    return;
                }
            }
        }
        
        Log::Add(Log::Level::Info, Body::TAG, "Starting body calibration sequence...");
        Log::GroupStart();
        for (size_t i = 0; i < static_cast<size_t>(LegIndex::COUNT); i++)
        {
            Leg* leg = &body->legs[i];
            Log::Add(Log::Level::Info, Body::TAG, "Calibrating leg %d/%d ...", i + 1, LegIndex::COUNT);
            // calibrate hip roll
            if ((err = leg->getHipRoll().getMotorController().startCalibration()) != Error::None)
            {
                Log::Add(Log::Level::Error, Body::TAG, "Failed to start calibration for hip roll of leg %d. Error: %d", i, static_cast<uint8_t>(err));
                vTaskDelete(nullptr);
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // give some time to start
            // wait for it to finish
            while (leg->getHipRoll().getMotorController().getCalibrationState() == MotorController::CalibrationState::CALIBRATING)
            {
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            if (leg->getHipRoll().getMotorController().getCalibrationState() != MotorController::CalibrationState::CALIBRATED)
            {
                Log::Add(Log::Level::Error, Body::TAG, "Hip roll of leg %d failed to calibrate.", i);
                vTaskDelete(nullptr);
                return;
            }

            // set to target 30deg position (to avoid leg collision during next calibrations)
            leg->getHipRoll().enable();
            leg->getHipRoll().setTarget(DEG_TO_RAD(30.f));
            vTaskDelay(pdMS_TO_TICKS(1000)); // wait to reach position
            // disable again for safety
            leg->getHipRoll().disable();

            // calibrate hip pitch
            if ((err = leg->getHipPitch().getMotorController().startCalibration()) != Error::None)
            {
                Log::Add(Log::Level::Error, Body::TAG, "Failed to start calibration for hip pitch of leg %d. Error: %d", i, static_cast<uint8_t>(err));
                vTaskDelete(nullptr);
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // give some time to start
            // wait for it to finish
            while (leg->getHipPitch().getMotorController().getCalibrationState() == MotorController::CalibrationState::CALIBRATING)
            {
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            if (leg->getHipPitch().getMotorController().getCalibrationState() != MotorController::CalibrationState::CALIBRATED)
            {
                Log::Add(Log::Level::Error, Body::TAG, "Hip pitch of leg %d failed to calibrate.", i);
                vTaskDelete(nullptr);
                return;
            }

            // set to target 30deg (or -120deg) position (to avoid leg collision during next calibrations)
            leg->getHipPitch().enable();
            leg->getHipPitch().setTarget(DEG_TO_RAD(i >= 2 ? -120.f : 30.f)); // back legs go negative, front legs positive
            vTaskDelay(pdMS_TO_TICKS(1000)); // wait to reach position
            // disable again for safety
            leg->getHipPitch().disable();

            // calibrate knee pitch
            if ((err = leg->getKneePitch().getMotorController().startCalibration()) != Error::None)
            {
                Log::Add(Log::Level::Error, Body::TAG, "Failed to start calibration for knee pitch of leg %d. Error: %d", i, static_cast<uint8_t>(err));
                vTaskDelete(nullptr);
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // give some time to start
            // wait for it to finish
            while (leg->getKneePitch().getMotorController().getCalibrationState() == MotorController::CalibrationState::CALIBRATING)
            {
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            if (leg->getKneePitch().getMotorController().getCalibrationState() != MotorController::CalibrationState::CALIBRATED)
            {
                Log::Add(Log::Level::Error, Body::TAG, "Knee pitch of leg %d failed to calibrate.", i);
                vTaskDelete(nullptr);
                return;
            }

            // retract the whole leg to let space for other legs to calibrate
            leg->getKneePitch().enable();
            leg->getHipPitch().enable();
            leg->getHipRoll().enable();
            leg->getKneePitch().setTarget(DEG_TO_RAD(135.f));
            leg->getHipPitch().setTarget(DEG_TO_RAD(40.f));
            leg->getHipRoll().setTarget(DEG_TO_RAD(0.f));
            vTaskDelay(pdMS_TO_TICKS(1000)); // wait to reach position
            // disable again for safety
            leg->getKneePitch().disable();
            leg->getHipPitch().disable();
            leg->getHipRoll().disable();
        }
        Log::GroupEnd();
        Log::Add(Log::Level::Info, Body::TAG, "Body calibration completed.");

        vTaskDelete(nullptr);
    }, "BodyCalib", 8192, this, tskIDLE_PRIORITY + 1, nullptr);

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