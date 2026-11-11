#include "locomotion/providers/MPDeterministic.hpp"
#include "Robot.hpp"

Status MPDeterministic::onCreate()
{
    rotation = Vec3f(0.f, 0.f, 0.f);
    pid_roll = PID(2.0f, 2.5f, 0.0f);
    pid_pitch = PID(2.0f, 2.5f, 0.0f);

    // Initialize kinematics engine with the right config
    kinematicsEngine = KinematicsEngine(KinematicsEngine::KinematicsConfig{
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

    return Status::Ok;
}

Status MPDeterministic::onReset()
{
    rotation = Vec3f(0.f, 0.f, 0.f);
    pid_roll.reset();
    pid_pitch.reset();

    return Status::Ok;
}

Status MPDeterministic::onDestroy()
{
    return Status::Ok;
}

Status MPDeterministic::onTick(float dt, const MotionState& state, const MotionIntent& input, MotionCommand& command)
{
    /// 1 - Dumb gait planning

    gaitPlanner.setVelocityCommand(input.body_vel.x, input.body_vel.y, input.body_vel.z);
    
    BodyCartesianState body_state;
    if (Status err = gaitPlanner.update(dt, body_state); err != Status::Ok)
    {
        return err;
    }

    /// 2 - IMU Stabilization

    const Vec3f& orient = state.orientation.toEulerAngles();
    rotation.x -= pid_roll.compute(0.f, orient.x, CONTROL_LOOP_DT_S) * 0.01f;
    rotation.y -= pid_pitch.compute(0.f, orient.y, CONTROL_LOOP_DT_S) * 0.01f;
    
    Vec3f position = Quatf::FromEulerAngles(Vec3f(-rotation.x, -rotation.y, 0.f)).rotate(input.body_pos);
    body_state.body_pos = position;
    body_state.body_rot = Vec3f(
        -rotation.x + input.body_rot.x,
        -rotation.y + input.body_rot.y,
        -rotation.z + input.body_rot.z
    );

    /// 3 - Compute IK

    if (Status err = kinematicsEngine.computeBodyIK(body_state, command); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}