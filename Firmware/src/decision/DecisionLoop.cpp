#include "decision/DecisionLoop.hpp"
#include "esp_log_timestamp.h"
#include "common/config.hpp"
#include "common/Log.hpp"

Status DecisionLoop::init()
{
    // ErrorHandle(ErrorStruct::DecisionLoopInitFailed);
    // FIXME : Maybe we should create the task here and just keep it suspended until start() is called
    return Status::Ok;
}

Status DecisionLoop::deinit()
{
    if (decision_loop_task != nullptr)
    {
        vTaskDelete(decision_loop_task);
        decision_loop_task = nullptr;
    }

    return Status::Ok;
}

Status DecisionLoop::start()
{
    loop_running = true;

    BaseType_t err = xTaskCreatePinnedToCore([](void* pvParams){
        DecisionLoop* decision_loop = (DecisionLoop*) pvParams;
        decision_loop->decision_loop();
    }, "DecisionLoop_Core0", 8192, this, tskIDLE_PRIORITY + 10, &decision_loop_task, CORE_BRAIN);

    if (err != pdPASS)
    {
        loop_running = false;
        LOG_ERROR(TAG, "Error creating DecisionLoop task");
        return Status::Unknown;
    }

    return Status::Ok;
}

Status DecisionLoop::stop()
{
    loop_running = false;

    // wait 2 decision cycles to be sure it's stopped
    vTaskDelay(pdMS_TO_TICKS(DECISION_LOOP_DT_MS * 2));

    return Status::Ok;
}

bool DecisionLoop::isRunning() const
{
    return loop_running;
}

Status DecisionLoop::setAutoLifeLevel(uint8_t level)
{
    if (level > AutoLifeLevel::Full)
    {
        return Status::InvalidParameters;
    }
    auto_life_level = level;
    // TODO : Implement real auto life logic and all
    return Status::Ok;
}

Status DecisionLoop::askBodyVelocity(float x_ms, float y_ms, float z_rads)
{
    last_ask_timestamp_ms = esp_log_timestamp();
    askedBodyVel = {x_ms, y_ms, z_rads};
    return Status::Ok;
}

Status DecisionLoop::askBodyRotation(float x_rad, float y_rad, float z_rad)
{
    last_ask_timestamp_ms = esp_log_timestamp();
    askedBodyRot = {x_rad, y_rad, z_rad};
    return Status::Ok;
}

Status DecisionLoop::askBodyPosition(float x_m, float y_m, float z_m)
{
    last_ask_timestamp_ms = esp_log_timestamp();
    askedBodyPos = {x_m, y_m, z_m};
    return Status::Ok;
}

Status DecisionLoop::askLegPosition(Leg::Id leg_id, float x_m, float y_m, float z_m, IPC::OverrideMode mode)
{
    last_ask_timestamp_ms = esp_log_timestamp();
    askedLegOverrides[(int) leg_id].mode = mode;
    askedLegOverrides[(int) leg_id].value_pos = {x_m, y_m, z_m};
    return Status::Ok;
}

Status DecisionLoop::askGaitType(GaitPlanner::GaitType gait)
{
    last_ask_timestamp_ms = esp_log_timestamp();
    askedGait = gait;
    return Status::Ok;
}

Status DecisionLoop::askJointAngle(Joint::Id joint_id, float angle_rad, IPC::OverrideMode mode)
{
    last_ask_timestamp_ms = esp_log_timestamp();
    if (joint_id >= Joint::Id::Count)
    {
        return Status::InvalidParameters;
    }
    askedJointAngles[(int) joint_id].mode = mode;
    askedJointAngles[(int) joint_id].value_rad = angle_rad;
    return Status::Ok;
}

void DecisionLoop::decision_loop()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(DECISION_LOOP_DT_MS);

    while (loop_running)
    {
        // Get the robot state from the Reflex core
        if (!IPC::getState(&state))
        {
            // LOG_WARNING(TAG, "Failed to get robot state from Reflex core");
        }

        // Update decision logic
        update(DECISION_LOOP_DT_MS / 1000.f, state);

        // Build final intent object (fusion between asked intents and own intent)
        // First fill it with the asked intents, and then override based on auto life level and safeguards
        IPC::ControlIntent final_intent;
        final_intent.body_vel = askedBodyVel;
        final_intent.body_rot = askedBodyRot;
        final_intent.body_pos = askedBodyPos;
        final_intent.gait = askedGait;
        for (int i = 0; i < (int) Joint::Id::Count; i++)
        {
            final_intent.joint_overrides[i] = askedJointAngles[i];
        }
        for (int i = 0; i < (int) Leg::Id::Count; i++)
        {
            final_intent.leg_overrides[i] = askedLegOverrides[i];
        }
        // Then override with auto life features
        fillIntent(final_intent, state);

        // Update timestamp and send intent to Reflex core
        final_intent.timestamp_ms = esp_log_timestamp();
        if (IPC::setIntent(final_intent) != Status::Ok)
        {
            LOG_WARNING(TAG, "Failed to send intent to Reflex core");
        }
        intent = final_intent; // store that for getters
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    if (decision_loop_task != nullptr)
    {
        vTaskDelete(nullptr);
        decision_loop_task = nullptr;
    }
}
