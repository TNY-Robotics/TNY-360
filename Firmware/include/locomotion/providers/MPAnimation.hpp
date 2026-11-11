#pragma once
#include "common/utils.hpp"
#include "locomotion/providers/MotionProvider.hpp"

#include <vector>
struct AnimationSequence
{
    struct AnimationFrame
    {
        float duration; // Blend duration from the previous frame to this frame in seconds
        BodyJointState jointState; // Joint state for this frame
    };

    std::vector<AnimationFrame> frames;
};

class MPAnimation : public MotionProvider
{
public:
    MPAnimation() = default;
    MPAnimation(const AnimationSequence& animation);
    virtual ~MPAnimation() = default;

protected:
    Status onCreate() override;
    
    Status onReset() override;
    
    Status onDestroy() override;

    Status onTick(float dt, const MotionState& state, const MotionIntent& input, MotionCommand& command) override;

private:
    AnimationSequence currentAnimation;

    int currentFrameIndex = 0;
    float frameTimeElapsed = 0.0f;

    AnimationSequence::AnimationFrame feedbackSnapshot; // Snapshot of the joint state at the start of the animation for blending
};