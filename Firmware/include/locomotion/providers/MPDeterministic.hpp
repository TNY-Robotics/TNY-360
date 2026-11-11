#pragma once
#include "common/utils.hpp"
#include "locomotion/providers/MotionProvider.hpp"
#include "locomotion/providers/MPDeterministic/GaitPlanner.hpp"
#include "locomotion/providers/MPDeterministic/KinematicsEngine.hpp"
#include "common/PID.hpp"

class MPDeterministic : public MotionProvider
{
public:
    constexpr static const char* TAG = "MPDeterministic";

    MPDeterministic() = default;
    virtual ~MPDeterministic() = default;

    // TODO : Add gait planning parameters and other

protected:
    Status onCreate() override;
    
    Status onReset() override;
    
    Status onDestroy() override;

    Status onTick(float dt, const MotionState& state, const MotionIntent& input, MotionCommand& command) override;

private:
    GaitPlanner gaitPlanner;
    KinematicsEngine kinematicsEngine;

    Vec3f rotation;
    PID pid_roll;
    PID pid_pitch;
};