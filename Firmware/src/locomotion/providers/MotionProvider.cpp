#include "locomotion/providers/MotionProvider.hpp"

Status MotionProvider::create()
{
    if (used) return Status::Ok; // already created

    if (Status err = onCreate(); err != Status::Ok)
    {
        return err;
    }

    used = true;
    return Status::Ok;
}

Status MotionProvider::reset()
{
    if (!used) return Status::InvalidState; // not created yet

    if (Status err = onReset(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}

Status MotionProvider::destroy()
{
    if (!used) return Status::InvalidState; // not created yet

    if (Status err = onDestroy(); err != Status::Ok)
    {
        return err;
    }

    used = false;
    return Status::Ok;
}

Status MotionProvider::tick(float dt, const MotionState& state, const MotionIntent& input, MotionCommand& command)
{
    if (!used) return Status::InvalidState; // not created yet

    if (Status err = onTick(dt, state, input, command); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}