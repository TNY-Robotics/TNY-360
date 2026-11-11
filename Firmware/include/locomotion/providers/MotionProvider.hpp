#pragma once
#include "common/utils.hpp"
#include "locomotion/IPC.hpp"
#include "locomotion/utils.hpp"

/// @brief Input state from the control loop to the motion provider
using MotionState = IPC::RobotState;

/// @brief Input intent from the control loop to the motion provider
using MotionIntent = IPC::ControlIntent;

/// @brief Output command from the motion provider to the control loop
using MotionCommand = BodyJointState;

class MotionProvider
{
public:
    MotionProvider() = default;
    virtual ~MotionProvider() = default;

    /**
     * @brief Get whether the motion provider is currently in use by the control loop.
     * @return True if the motion provider is in use, false otherwise.
     */
    bool isUsed() const { return used; }

    /**
     * @brief Create the motion provider.
     * @return Status code indicating success or failure of the operation.
     */
    Status create();

    /**
     * @brief Reset the motion provider (after fall recovery for example).
     * @return Status code indicating success or failure of the operation.
     */
    Status reset();

    /**
     * @brief Destroy the motion provider.
     * @return Status code indicating success or failure of the operation.
     */
    Status destroy();

    /**
     * @brief Execute the motion provider to update the motion command based on the current state and input.
     * @param intent Current motion intent (input from the control loop).
     * @param dt Delta time in seconds since the last tick.
     * @param state Current motion state (state of the robot).
     * @param command Output motion command to be filled by the provider.
     * @return Status code indicating success or failure of the operation.
     */
    Status tick(float dt, const MotionState& state, const MotionIntent& input, MotionCommand& command);
    
protected:
    /**
     * @brief Called when the motion provider is created. Use this to initialize any resources.
     * @return Status code indicating success or failure of the operation.
     */
    virtual Status onCreate() = 0;

    /**
     * @brief Called when the motion provider is reset (after fall recovery for example).
     * @return Status code indicating success or failure of the operation.
     */
    virtual Status onReset() = 0;

    /**
     * @brief Called when the motion provider is destroyed. Use this to clean up any resources.
     * @return Status code indicating success or failure of the operation.
     */
    virtual Status onDestroy() = 0;

    /**
     * @brief Called on each control loop tick to update the motion command, based on the current state and input.
     * @param dt Delta time in seconds since the last tick.
     * @param state Current motion state (state of the robot).
     * @param input Current motion intent (input from the control loop).
     * @param command Output motion command to be filled by the provider.
     * @return Status code indicating success or failure of the operation.
     */
    virtual Status onTick(float dt, const MotionState& state, const MotionIntent& input, MotionCommand& command) = 0;

private:
    /// @brief Whether the motion provider is currently in use by the control loop.
    bool used = false;
};