#include "common/analysis/PerfMonitor.hpp"
#include "locomotion/ControlLoop.hpp"
#include "esp_log_timestamp.h"
#include "locomotion/IPC.hpp"
#include "common/RPC.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"

#include "System.Error.hpp"

/// @brief Default motion command to be used when no motion provider is set.
MotionCommand COMMAND_DEFAULT = {
    .leg_joints = {
        { 0.000f, -0.785f, 1.570f, },  // FRONT_LEFT (HIP_ROLL, HIP_PITCH, KNEE_PITCH)
        { 0.000f, -0.785f, 1.570f, },  // BACK_LEFT (HIP_ROLL, HIP_PITCH, KNEE_PITCH)
        { 0.000f, -0.785f, 1.570f, },  // BACK_RIGHT (HIP_ROLL, HIP_PITCH, KNEE_PITCH)
        { 0.000f, -0.785f, 1.570f, },  // FRONT_RIGHT (HIP_ROLL, HIP_PITCH, KNEE_PITCH)
    },
    .ear_r_rad = 0.785f,
    .ear_l_rad = 0.785f,
};

Status ControlLoop::control_task()
{
    static bool watchdog_active = false;

    /*** UPDATE CONTROL INTENT ***/
    // update from IPC
    static IPC::ControlIntent intent;
    IPC::getIntent(&intent);

    // security timestamp
    uint32_t current_time = esp_log_timestamp();
    if ((current_time - intent.timestamp_ms) > CONTROL_INTENT_WATCHDOG_MS)
    {
        // Brain is overloaded or crashed. Force robot stop.
        intent.body_vel = Vec3f(0.f, 0.f, 0.f);

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

    /*** STATE ESTIMATION - READ ALL SENSORS ***/

    // Read the ADC channels
    if (Status err = AnalogDriver::ReadAllChannels(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Error reading all ADC channels");
    }

    // Estimate body state from new IMU and Analog data (calls Legs, Joint, IMU estimateState functions)
    if (Status err = Robot::GetInstance().getBody().estimateState(CONTROL_LOOP_DT_S); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Error estimating body state");
        Error::RegisterErrorEvent(System::ErrorEventStartFailed());
    }

    /// Store the state in the IPC to be read by the Brain core
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
    for (int i = 0; i < (int) Leg::Id::Count; i++)
    {
        state.grounded[i] = Robot::GetInstance().getBody().getLeg(static_cast<Leg::Id>(i)).isGrounded();
    }
    IMU* imu = Robot::GetInstance().getBody().getIMU();
    if (imu != nullptr) state.orientation = Quatf(imu->getOrientation());
    else state.orientation = Quatf::Identity(); // No IMU, default value
    IPC::setState(state);

    /*** MOTION PROVIDER EXECUTION ***/

    MotionCommand command = COMMAND_DEFAULT;
    if (motion_provider != nullptr)
    {
        // Execute the motion provider to get the desired joint angles
        if (Status err = motion_provider->tick(CONTROL_LOOP_DT_S, state, intent, command); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Error executing motion provider, destroying it");
            motion_provider->destroy();
            delete motion_provider;
            motion_provider = nullptr;
            Robot::GetInstance().getBody().disable();
        }
    }

    /*** SEND EVERYTHING ***/

    // Update the body (this updates all joints in the body)
    if (Status err = Robot::GetInstance().getBody().applyCommand(command, CONTROL_LOOP_DT_S); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Failed to apply command in control task with error: %s", ErrorToString(err));
        return err;
    }
    
    // Send the new motor values
    if (Status err = MotorDriver::SendData(); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Error sending MotorDriver data with error: %s", ErrorToString(err));
        Error::RegisterErrorEvent(System::ErrorEventStartFailed());
        return err;
    }

    /*** OTHER CORE1 JOBS ***/

    // All done, we can execute pending jobs if there's any (Handle RPC Calls)
    RPC::Process_Core1();

    return Status::Ok;
}