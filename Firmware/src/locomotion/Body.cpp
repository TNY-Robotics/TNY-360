#include "locomotion/Body.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "drivers/PowerDriver.hpp"
#include "common/RPC.hpp"
#include "locomotion/IPC.hpp"
#include "common/RPC.hpp"
#include "Robot.hpp"

constexpr float JOINT_POSE_REST[12] = {
    // HIP_ROLL     , HIP_PITCH         , KNEE_PITCH
    DEG_TO_RAD(0.0f), DEG_TO_RAD(-60.0f), DEG_TO_RAD(150.0f),  // FRONT_LEFT
    DEG_TO_RAD(0.0f), DEG_TO_RAD(-60.0f), DEG_TO_RAD(150.0f),  // BACK_LEFT
    DEG_TO_RAD(0.0f), DEG_TO_RAD(-60.0f), DEG_TO_RAD(150.0f),  // BACK_RIGHT
    DEG_TO_RAD(0.0f), DEG_TO_RAD(-60.0f), DEG_TO_RAD(150.0f)   // FRONT_RIGHT
};

constexpr float JOINT_POSE_STAND[12] = {
    // HIP_ROLL     , HIP_PITCH         , KNEE_PITCH
    DEG_TO_RAD(5.0f), DEG_TO_RAD(-50.0f), DEG_TO_RAD(100.0f),  // FRONT_LEFT
    DEG_TO_RAD(5.0f), DEG_TO_RAD(-50.0f), DEG_TO_RAD(100.0f),  // BACK_LEFT
    DEG_TO_RAD(5.0f), DEG_TO_RAD(-50.0f), DEG_TO_RAD(100.0f),  // BACK_RIGHT
    DEG_TO_RAD(5.0f), DEG_TO_RAD(-50.0f), DEG_TO_RAD(100.0f)   // FRONT_RIGHT
};

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


float Body::enableSmooth()
{
    const float VELOCITY_REST = Joint::MAX_VELOCITY_RAD_S * 0.05f; // 5% of max speed
    const float VELOCITY_STAND = Joint::MAX_VELOCITY_RAD_S * 0.1f; // 10% of max speed
    
    // First estimate movement time
    auto estimateJointTravelTime = [](float angle_from, float angle_to, float max_velocity_rad_s)
    {
        float angle_diff = fabsf(angle_to - angle_from);
        return angle_diff / max_velocity_rad_s;
    };

    // From current to rest position
    float current_to_rest_time = 0.0f;
    for (size_t i = 0; i < 12; i++)
    {
        if (Joint* joint = Joint::GetJoint(static_cast<Joint::Id>(i)); joint != nullptr)
        {
            float current_angle;
            if (joint->getFeedback(current_angle) != Status::Ok)
            {
                LOG_ERROR(TAG, "Failed to get feedback for joint %d", i);
                return -1;
            }
            float target_angle = JOINT_POSE_REST[i];
            current_to_rest_time = std::max(current_to_rest_time, estimateJointTravelTime(current_angle, target_angle, VELOCITY_REST));
        }
    }

    // From rest to stand position
    float rest_to_stand_time = 0.0f;
    for (size_t i = 0; i < 12; i++)
    {
        float rest_angle = JOINT_POSE_REST[i];
        float stand_angle = JOINT_POSE_STAND[i];
        rest_to_stand_time = std::max(rest_to_stand_time, estimateJointTravelTime(rest_angle, stand_angle, VELOCITY_STAND));
    }

    // total time
    float total_estimated_time = current_to_rest_time + rest_to_stand_time + 1.0f; // +1s buffer for safety

    struct Params
    {
        float rest_time;
        float stand_time;
        float velocity_rest;
        float velocity_stand;
    };

    // Create the FreeRTOS task to handle the smooth enable process
    BaseType_t task_result = xTaskCreatePinnedToCore(
        [](void* param)
        {
            Params* params = static_cast<Params*>(param);

            // Set target to rest position
            DecisionLoop& decision_loop = Robot::GetInstance().getDecisionLoop();
            for (size_t i = 0; i < 12; i++)
            {
                decision_loop.askJointAngle(static_cast<Joint::Id>(i), JOINT_POSE_REST[i]);
            }

            // Enable all joints at rest speed
            RPC::ExecuteThreadSafe<bool>([params]() {
                for (size_t i = 0; i < 12; i++)
                {
                    if (Joint* joint = Joint::GetJoint(static_cast<Joint::Id>(i)); joint != nullptr)
                    {
                        joint->setVelocity(params->velocity_rest);
                        joint->enable();
                    }
                }
                return true;
            }, [](bool res){});  

            // Wait for the rest movement to complete
            vTaskDelay(pdMS_TO_TICKS(params->rest_time * 1000));

            // Set target to stand position
            for (size_t i = 0; i < 12; i++)
            {
                decision_loop.askJointAngle(static_cast<Joint::Id>(i), JOINT_POSE_STAND[i]);
            }

            // Set all joints to stand speed (with movement sync, so some of them will be slower)

            // first get the biggest movement difference
            float max_mov_diff = 0.0f;
            for (size_t i = 0; i < 12; i++) max_mov_diff = std::max(max_mov_diff, fabsf(JOINT_POSE_STAND[i] - JOINT_POSE_REST[i]));

            // then set the velocity for each joint
            RPC::ExecuteThreadSafe<bool>([params, max_mov_diff]() {
                for (size_t i = 0; i < 12; i++)
                {
                    if (Joint* joint = Joint::GetJoint(static_cast<Joint::Id>(i)); joint != nullptr)
                    {
                        float mov_diff = fabsf(JOINT_POSE_STAND[i] - JOINT_POSE_REST[i]);
                        joint->setVelocity(params->velocity_stand * (mov_diff / max_mov_diff));
                    }
                }
                return true;
            }, [](bool res){});

            // Wait for the stand movement to complete
            vTaskDelay(pdMS_TO_TICKS(params->stand_time * 1000));

            // Reset all joints to max speed
            RPC::ExecuteThreadSafe<bool>([](){
                for (size_t i = 0; i < 12; i++)
                {
                    if (Joint* joint = Joint::GetJoint(static_cast<Joint::Id>(i)); joint != nullptr)
                    {
                        joint->setVelocity(Joint::MAX_VELOCITY_RAD_S);
                    }
                }
                return true;
            }, [](bool res){});

            vTaskDelay(pdMS_TO_TICKS(500)); // wait a bit to ensure robot is stable and all

            // Remove joint override (let joint control be handled by the control loop as normal)
            for (size_t i = 0; i < 12; i++)
            {
                decision_loop.askJointAngle(static_cast<Joint::Id>(i), 0, IPC::OverrideMode::None);
            }

            // Clean up and delete
            delete params;
            vTaskDelete(nullptr);
        },
        "SmoothEnableTask",
        2048,
        new Params{current_to_rest_time, rest_to_stand_time, VELOCITY_REST, VELOCITY_STAND},
        tskIDLE_PRIORITY + 1,
        nullptr,
        CORE_BRAIN
    );
    if (task_result != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create smooth enable task");
        return -1;
    }

    return total_estimated_time;
}

float Body::disableSmooth()
{
    DecisionLoop& decision_loop = Robot::GetInstance().getDecisionLoop();

    // Get current position
    float current_position[12];
    for (size_t i = 0; i < 12; i++)
    {
        current_position[i] = decision_loop.getRobotState().joints[i].feedback_angle_rad;
    }

    // Get time towards rest position
    auto estimateJointTravelTime = [](float angle_from, float angle_to, float max_velocity_rad_s)
    {
        float angle_diff = fabsf(angle_to - angle_from);
        return angle_diff / max_velocity_rad_s;
    };

    float current_to_rest_time = 0.0f;
    for (size_t i = 0; i < 12; i++)
    {
        current_to_rest_time = std::max(current_to_rest_time, estimateJointTravelTime(current_position[i], JOINT_POSE_REST[i], Joint::MAX_VELOCITY_RAD_S * 0.1f));
    }
    float total_estimated_time = current_to_rest_time + 1.0f; // +1s buffer for safety

    // Start the smooth disable task
    BaseType_t task_result = xTaskCreatePinnedToCore(
        [](void* param)
        {
            DecisionLoop& decision_loop = Robot::GetInstance().getDecisionLoop();
            float current_to_rest_time = *static_cast<float*>(param);

            // Set target to rest position
            for (size_t i = 0; i < 12; i++)
            {
                decision_loop.askJointAngle(static_cast<Joint::Id>(i), JOINT_POSE_REST[i]);
            }

            // Enable all joints at rest speed
            RPC::ExecuteThreadSafe<bool>([]() {
                for (size_t i = 0; i < 12; i++)
                {
                    if (Joint* joint = Joint::GetJoint(static_cast<Joint::Id>(i)); joint != nullptr)
                    {
                        joint->setVelocity(Joint::MAX_VELOCITY_RAD_S * 0.1f);
                        joint->enable();
                    }
                }
                return true;
            }, [](bool res){});  

            // Wait for the rest movement to complete
            vTaskDelay(pdMS_TO_TICKS(current_to_rest_time * 1000));

            // Disable all joints
            RPC::ExecuteThreadSafe<bool>([]() {
                for (size_t i = 0; i < 12; i++)
                {
                    if (Joint* joint = Joint::GetJoint(static_cast<Joint::Id>(i)); joint != nullptr)
                    {
                        joint->disable();
                    }
                }
                return true;
            }, [](bool res){});

            // Clean up and delete
            delete static_cast<float*>(param);
            vTaskDelete(nullptr);
        },
        "SmoothDisableTask",
        2048,
        new float(current_to_rest_time),
        tskIDLE_PRIORITY + 1,
        nullptr,
        CORE_BRAIN
    );

    if (task_result != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create smooth disable task");
        return -1;
    }

    return total_estimated_time;
}