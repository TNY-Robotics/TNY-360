#include "locomotion/ControlLoop.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log_timestamp.h"
#include "Robot.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "common/RPC.hpp"
#include "locomotion/IPC.hpp"
#include "drivers/AnalogDriver.hpp"
#include "drivers/MotorDriver.hpp"
#include "drivers/IMUDriver.hpp"

// Perf monitoring : Remove this when control loop is optimized and stable
PerfMonitor perf_global;
PerfMonitor perf_reader;
PerfMonitor perf_imu;
PerfMonitor perf_estimation;
PerfMonitor perf_gait;
PerfMonitor perf_ik;
PerfMonitor perf_command;
PerfMonitor perf_driver;
uint16_t perf_counter = 0;

TaskHandle_t timer_task_handle;
bool running = false;

static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    vTaskNotifyGiveFromISR(timer_task_handle, &high_task_awoken);
    return (high_task_awoken == pdTRUE); // return true if a higher priority task was awoken
}

ControlLoop::ControlLoop()
    : initialized(false)
{
}

Error ControlLoop::init()
{
    // FIXME : It would really be better if we made sure everything is in DRAM
    //         to avoid cache issues and delays in the control loop when brain core is doing heavy operations

    if (initialized) return Error::None;

    // Initialize kinematics engine with the right config
    kinematics_engine = KinematicsEngine(KinematicsEngine::KinematicsConfig{
        .hip_shift_x = HIP_POS_X_M,
        .hip_shift_y = HIP_POS_Y_M,
        .hip_offset = HIP_OFFSET_M,
        .length_thigh = LEG_THIGH_LENGTH_M,
        .length_calf = LEG_CALF_LENGTH_M,
        .leg_inverted = {
            Robot::GetInstance().getBody().getLeg(Leg::Id::FrontLeft).isInverted(),
            Robot::GetInstance().getBody().getLeg(Leg::Id::BackLeft).isInverted(),
            Robot::GetInstance().getBody().getLeg(Leg::Id::BackRight).isInverted(),
            Robot::GetInstance().getBody().getLeg(Leg::Id::FrontRight).isInverted(),
        },
    });

    // Create the control loop task ON REFLEX CORE
    BaseType_t err = xTaskCreatePinnedToCore([](void* PvParams){
        ControlLoop* control_loop = static_cast<ControlLoop*>(PvParams);
        running = true;
        
        // Main loop
        while (running)
        {
            // wait for timer interrupt notification
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            control_loop->control_task();
        }

        // clean up and delete task
        running = false;
        if (timer_task_handle != nullptr)
        {
            vTaskDelete(nullptr);
            timer_task_handle = nullptr;
        }
    },  "timer_task", 8192, this, configMAX_PRIORITIES - 1, &timer_task_handle, CORE_REFLEX);

    if (err != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create timer task");
        ErrorHandle(ErrorStruct::ControlLoopInitFailed);
        return Error::SoftwareFailure;
    }

    // Configure the timer
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION,
        .intr_priority = 0, // Let driver choose a low priority
        .flags = {
            .intr_shared = false, // Don't share the interrupt
            .allow_pd = false, // Don't allow power down
        },
    };
    if (esp_err_t err = gptimer_new_timer(&timer_config, &timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_new_timer failed");
        return Error::SoftwareFailure;
    }

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb, // link the alarm callback
    };
    if (esp_err_t err = gptimer_register_event_callbacks(timer, &cbs, NULL); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_register_event_callbacks failed");
        return Error::SoftwareFailure;
    }

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = TIMER_ALARM_COUNT,
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true,
        },
    };
    if (esp_err_t err = gptimer_set_alarm_action(timer, &alarm_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_set_alarm_action failed");
        return Error::SoftwareFailure;
    }

    // Enable the timer
    if (esp_err_t err = gptimer_enable(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "gptimer_enable failed");
        return Error::SoftwareFailure;
    }

    initialized = true;
    return Error::None;
}

Error ControlLoop::start()
{
    if (esp_err_t err = gptimer_start(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error starting Control Loop timer : 0x%0X", err);
        return Error::HardwareFailure;
    }

    return Error::None;
}


Error ControlLoop::stop()
{
    if (esp_err_t err = gptimer_stop(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error stopping Control Loop timer : 0x%0X", err);
        return Error::HardwareFailure;
    }

    return Error::None;
}

bool ControlLoop::isRunning() const
{
    return running;
}


Error ControlLoop::deinit()
{
    if (!initialized) return Error::None;

    running = false; // ask the control loop to end

    // wait 2 control cycle to be sure the control loop is stopped
    vTaskDelay(pdMS_TO_TICKS(CONTROL_LOOP_DT_MS*2));

    if (timer_task_handle != nullptr)
    {
        vTaskDelete(timer_task_handle);
        timer_task_handle = nullptr;
    }

    if (esp_err_t err = gptimer_disable(timer); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Error disabling Control Loop gptimer ; 0x%0X", err);
        return Error::HardwareFailure;
    }

    initialized = false;
    return Error::None;
}

Error ControlLoop::control_task()
{
    perf_global.start();

    static bool watchdog_active = false;

    /*** UPDATE CONTROL INTENT ***/
    // update from IPC
    static IPC::ControlIntent intent;
    bool new_intent = IPC::getIntent(&intent);

    // security timestamp
    uint32_t current_time = esp_log_timestamp();
    if ((current_time - intent.timestamp_ms) > CONTROL_INTENT_WATCHDOG_MS)
    {
        // Brain is overloaded or crashed. Force robot stop.
        intent.body_vel = Vec3f(0.f, 0.f, 0.f);
        intent.gait = GaitPlanner::GaitType::Walk;

        if (!watchdog_active) {
            LOG_WARNING(TAG, "Control intent watchdog triggered. Stop overthinking!");
            watchdog_active = true;
        }
    }
    else
    {
        if (watchdog_active) {
            LOG_INFO(TAG, "Brain is back online!");
            watchdog_active = false;
        }
    }

    /*** 1 - STATE ESTIMATION - READ ALL SENSORS ***/

    // Read the ADC channels and IMU Data
    perf_reader.start();
    if (Error err = AnalogDriver::ReadAllChannels(); err != Error::None)
    {
        LOG_ERROR(TAG, "Error reading all ADC channels");
    }
    perf_reader.stop();

    perf_imu.start();
    if (Error err = IMUDriver::ReadData(); err != Error::None)
    {
        LOG_ERROR(TAG, "Error reading data from IMU");
    }
    perf_imu.stop();

    // Estimate body state from new IMU and Analog data (calls Legs, Joint, IMU estimateState functions)
    perf_estimation.start();
    if (Error err = Robot::GetInstance().getBody().estimateState(CONTROL_LOOP_DT_S); err != Error::None)
    {
        LOG_ERROR(TAG, "Error estimating body state");
    }
    perf_estimation.stop();

    /// Store the state in the IPC to be read by the Brain core (we don't check return error here, no time to manage them)
    IPC::RobotState state;
    state.timestamp_ms = current_time;
    for (int i = 0; i < (int) Joint::Id::Count; i++)
    {
        Joint::Id joint_id = static_cast<Joint::Id>(i);
        Joint* joint = Joint::GetJoint(joint_id);
        if (joint == nullptr) continue;
        joint->getTarget(state.joints[i].target_angle_rad);
        joint->getFeedback(state.joints[i].feedback_angle_rad);
        joint->getPrediction(state.joints[i].model_angle_rad);
        joint->getPosition(state.joints[i].estimated_angle_rad);
    }
    state.body_orientation = Robot::GetInstance().getBody().getIMU().getOrientation();
    state.imu_down_vector = Robot::GetInstance().getBody().getIMU().getDownVector();
    IPC::setState(state);

    /*** 2 - RUN CARTESIAN CONTROL (USING BRAIN CONTROL INTENT) ***/

    // build the initial state
    BodyCartesianState cartesian_state;
    cartesian_state.body_pos = intent.body_pos;
    cartesian_state.body_rot = intent.body_rot;
    for (int i = 0; i < (int) Leg::Id::Count; i++)
    {
        Leg::Id leg_id = static_cast<Leg::Id>(i);
        cartesian_state.legs[i].is_grounded = Robot::GetInstance().getBody().getLeg(leg_id).isGrounded();
    }

    // Gait planner (ideal movement)
    if (new_intent)
    {
        gait_planner.setVelocityCommand(intent.body_vel.x, intent.body_vel.y, intent.body_vel.z);
        GaitPlanner::GaitConfig current_config = gait_planner.getConfig();
        if (current_config.gait_type != intent.gait)
        {
            current_config.gait_type = intent.gait;
            gait_planner.setGaitConfig(current_config);
        }
    }
    perf_gait.start();
    gait_planner.update(CONTROL_LOOP_DT_S, cartesian_state);
    perf_gait.stop();

    // Animation override (leg override + body override)
    for (uint8_t leg_id = 0; leg_id < (uint8_t) Leg::Id::Count; leg_id++)
    {
        IPC::LegOverride& leg_override = intent.leg_overrides[leg_id];
        if (leg_override.mode == IPC::OverrideMode::None) continue;
        else if (leg_override.mode == IPC::OverrideMode::Absolute)
        {
            cartesian_state.legs[leg_id].target_pos = leg_override.value_pos;
        }
        else if (leg_override.mode == IPC::OverrideMode::Relative)
        {
            cartesian_state.legs[leg_id].target_pos += leg_override.value_pos;
        }
    }
    IPC::BodyOverride& body_override = intent.body_override;
    if (body_override.mode == IPC::OverrideMode::Absolute)
    {
        cartesian_state.body_pos = body_override.value_pos;
        cartesian_state.body_rot = body_override.value_rot;
    }
    else if (body_override.mode == IPC::OverrideMode::Relative)
    {
        cartesian_state.body_pos += body_override.value_pos;
        cartesian_state.body_rot += body_override.value_rot;
    }

    /*** 3 - CONVERT CARTESIAN CONTROL TO JOINT CONTROL ***/

    BodyJointState joint_state;

    // IK
    perf_ik.start();
    if (Error err = kinematics_engine.computeBodyIK(cartesian_state, joint_state); err != Error::None)
    {
        LOG_ERROR(TAG, "KinematicsEngine failed to calculate body IK with error: %s", ErrorToString(err));
        return err;
    }
    perf_ik.stop();


    /*** 4 - RUN JOINT CONTROL (USING BRAIN CONTROL INTENT) ***/

    // Joint Override
    for (uint8_t leg_id = 0; leg_id < (uint8_t) Leg::Id::Count; leg_id++)
    {
        for (uint8_t leg_joint_id = 0; leg_joint_id < (uint8_t) Leg::JointId::Count; leg_joint_id++)
        {
            uint8_t joint_id = leg_id * (uint8_t) Leg::JointId::Count + leg_joint_id;

            IPC::JointOverride& joint_override = intent.joint_overrides[joint_id];
            if (joint_override.mode == IPC::OverrideMode::None) continue;
            else if (joint_override.mode == IPC::OverrideMode::Absolute)
            {
                joint_state.leg_joints[leg_id].joint_angles_rad[leg_joint_id] = joint_override.value_rad;
            }
            else if (joint_override.mode == IPC::OverrideMode::Relative)
            {
                joint_state.leg_joints[leg_id].joint_angles_rad[leg_joint_id] += joint_override.value_rad;
            }
        }
    }
    // ears
    IPC::JointOverride& ear_l_override = intent.joint_overrides[(uint8_t) Joint::Id::EarLeft];
    if (ear_l_override.mode == IPC::OverrideMode::Absolute) joint_state.ear_l_rad = ear_l_override.value_rad;
    else if (ear_l_override.mode == IPC::OverrideMode::Relative) joint_state.ear_l_rad += ear_l_override.value_rad;
    IPC::JointOverride& ear_r_override = intent.joint_overrides[(uint8_t) Joint::Id::EarRight];
    if (ear_r_override.mode == IPC::OverrideMode::Absolute) joint_state.ear_r_rad = ear_r_override.value_rad;
    else if (ear_r_override.mode == IPC::OverrideMode::Relative) joint_state.ear_r_rad += ear_r_override.value_rad;

    /*** 5 - SEND EVERYTHING ***/

    // Update the body (this updates all joints in the body)
    perf_command.start();
    if (Error err = Robot::GetInstance().getBody().applyCommand(joint_state, CONTROL_LOOP_DT_S); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to apply command in control task with error: %s", ErrorToString(err));
        // return err;
    }
    perf_command.stop();
    
    // Send the new motor values
    perf_driver.start();
    if (Error err = MotorDriver::SendData(); err != Error::None)
    {
        LOG_ERROR(TAG, "Error sending MotorDriver data with error: %s", ErrorToString(err));
    }
    perf_driver.stop();

    /*** 6 - OTHER CORE1 JOBS ***/

    // All done, we can execute pending jobs if there's any (Handle RPC Calls)
    RPC::Process_Core1();
    
    // Performance tracking (as lightweight as possible)
    perf_global.stop();
    if (perf_counter++ == CONTROL_LOOP_FREQ_HZ)
    {
        float ms_global = perf_global.get_avg_ms();
        float ms_reader = perf_reader.get_avg_ms();
        float ms_imu = perf_imu.get_avg_ms();
        float ms_estimation = perf_estimation.get_avg_ms();
        float ms_gait = perf_gait.get_avg_ms();
        float ms_ik = perf_ik.get_avg_ms();
        float ms_command = perf_command.get_avg_ms();
        float ms_driver = perf_driver.get_avg_ms();

        LOG_DEBUG(TAG, "Control loop perfs:\n| Global | Reader |  IMU   | Estim  |  Gait  |   IK   |Command | Driver |\n|--------|--------|--------|--------|--------|--------|--------|--------|\n| %5.2f  | %5.2f  | %5.2f  | %5.2f  | %5.2f  | %5.2f  | %5.2f  | %5.2f  |\n", ms_global, ms_reader, ms_imu, ms_estimation, ms_gait, ms_ik, ms_command, ms_driver);
        perf_counter = 0;
        perf_global.reset();
        perf_reader.reset();
        perf_imu.reset();
        perf_estimation.reset();
        perf_gait.reset();
        perf_ik.reset();
        perf_command.reset();
        perf_driver.reset();
    }

    return Error::None;
}