#include "locomotion/providers/MPAnimation.hpp"
#include "common/Lerp.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"

MPAnimation::MPAnimation(const AnimationSequence& animation)
    : currentAnimation(animation)
{
}

MPAnimation* MPAnimation::onFinished(std::function<void()> callback)
{
    finishedCallback = callback;
    return this;
}

Status MPAnimation::onCreate()
{
    return onReset();
}

Status MPAnimation::onReset()
{
    currentFrameIndex = 0;
    frameTimeElapsed = 0.0f;

    // Capture the current joint state as the feedback snapshot for blending
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            Joint& joint = Robot::GetInstance().getBody().getLeg(static_cast<Leg::Id>(i)).getJoint(static_cast<Leg::JointId>(j));
            joint.getFeedback(feedbackSnapshot.jointState.leg_joints[i].joint_angles_rad[j]);
        }
    }

    return Status::Ok;
}

Status MPAnimation::onDestroy()
{
    return Status::Ok;
}

Status MPAnimation::onTick(float dt, const MotionState& state, const MotionIntent& input, MotionCommand& command)
{
    if (currentFrameIndex >= currentAnimation.frames.size())
    {
        if (finishedCallback) finishedCallback();
        return Status::OutOfBounds; // No more frames to process
    }

    const auto& currentFrame = currentAnimation.frames[currentFrameIndex];
    const auto& previousFrame = (currentFrameIndex > 0) ? currentAnimation.frames[currentFrameIndex - 1] : feedbackSnapshot;

    float blendFactor = (currentFrame.duration > 0.0f) ? (frameTimeElapsed / currentFrame.duration) : 1.0f;

    command.ear_r_rad = Lerp::EaseInOut(previousFrame.jointState.ear_r_rad, currentFrame.jointState.ear_r_rad, blendFactor);
    command.ear_l_rad = Lerp::EaseInOut(previousFrame.jointState.ear_l_rad, currentFrame.jointState.ear_l_rad, blendFactor);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            command.leg_joints[i].joint_angles_rad[j] = Lerp::EaseInOut(
                previousFrame.jointState.leg_joints[i].joint_angles_rad[j],
                currentFrame.jointState.leg_joints[i].joint_angles_rad[j],
                blendFactor
            );
        }
    }

    frameTimeElapsed += dt;
    if (frameTimeElapsed >= currentFrame.duration)
    {
        currentFrameIndex++;
        frameTimeElapsed = 0.0f;
    }
    return Status::Ok;
}