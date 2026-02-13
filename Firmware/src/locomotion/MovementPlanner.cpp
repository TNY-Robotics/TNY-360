#include "locomotion/MovementPlanner.hpp"
#include "Robot.hpp"
#include <cmath>

// Offsets for typical trot gait (FL+BR together, FR+BL together)
// FL(0), FR(1), BL(2), BR(3)
const float LEG_PHASE_OFFSETS[4] = { 0.0f, 0.5f, 0.5f, 0.0f }; 

MovementPlanner::MovementPlanner()
{
    // set default leg positions (relative to body center)
    leg_default_pos[0] = Vec3f( WALK_LEG_SPREAD_X_MM,  WALK_LEG_SPREAD_Y_MM, 0); // FL
    leg_default_pos[1] = Vec3f( WALK_LEG_SPREAD_X_MM, -WALK_LEG_SPREAD_Y_MM, 0); // FR
    leg_default_pos[2] = Vec3f(-WALK_LEG_SPREAD_X_MM,  WALK_LEG_SPREAD_Y_MM, 0); // BL
    leg_default_pos[3] = Vec3f(-WALK_LEG_SPREAD_X_MM, -WALK_LEG_SPREAD_Y_MM, 0); // BR
}

void MovementPlanner::setVelocityCommand(float v_x, float v_y, float omega)
{
    // Store the velocity command for use in the update loop
    cmd_vel_linear = Vec2f(v_x, v_y);
    cmd_vel_angular = omega;
}

Error MovementPlanner::update()
{
    if (cmd_vel_linear.x == 0 && cmd_vel_linear.y == 0 && cmd_vel_angular == 0)
    {
        if (is_moving) // just stopped, reset legs
        {
            Body& body = Robot::GetInstance().getBody();
            for (int i = 0; i < 4; i++) {
                body.setFeetPosition((Body::LegIndex)i, leg_default_pos[i]);
            }
        }
        is_moving = false;
        return Error::None;
    }
    if (!is_moving) // just started moving, reset gait phase
    {
        main_gait_phase = 0.0f;
        is_moving = true;
    }

    // Advance the main gait phase
    main_gait_phase += (CONTROL_LOOP_DT_S * gait_config.step_freq_hz);
    if (main_gait_phase >= 1.0f) main_gait_phase -= 1.0f;

    Body& body = Robot::GetInstance().getBody();
    
    // Duration of phases in seconds (for distance calculations)
    float cycle_time_s = 1.0f / gait_config.step_freq_hz;
    float stance_time_s = cycle_time_s * gait_config.duty_factor;
    // float swing_time_s = cycle_time_s * (1.0f - config.duty_factor);

    for (int i = 0; i < 4; i++) {
        // local leg phase
        float leg_phase = main_gait_phase + LEG_PHASE_OFFSETS[i];
        if (leg_phase >= 1.0f) leg_phase -= 1.0f;

        // [0 .. swing_ratio] = Swing, [swing_ratio .. 1.0] = Stance
        float swing_ratio = 1.0f - gait_config.duty_factor;

        // I KNOW THIS ISNT MATHEMATICALLY RIGHT
        // But it saves the computation feet rotations,
        // and introduces an error way smaller than the mecanical backlash and all, so ...
        Vec2f leg_vel_linear = cmd_vel_linear;
        leg_vel_linear.x += -cmd_vel_angular * leg_default_pos[i].y;
        leg_vel_linear.y +=  cmd_vel_angular * leg_default_pos[i].x;

        Vec3f target_pos;

        if (leg_phase < swing_ratio) // --- SWING PHASE (In the air) ---
        {
            float t_swing = leg_phase / swing_ratio; 

            Vec2f stance_dist = leg_vel_linear * stance_time_s;
            
            // Liftoff point, start of the swing trajectory
            Vec3f start = leg_default_pos[i];
            start.x -= stance_dist.x * 0.5f; 
            start.y -= stance_dist.y * 0.5f;

            // Touchdown point, end of the swing trajectory
            Vec3f end = leg_default_pos[i];
            end.x += stance_dist.x * 0.5f;
            end.y += stance_dist.y * 0.5f;

            // cycloidal smoothing (smaller acceleration spikes)
            float smooth_t = (1.0f - std::cos(PI * t_swing)) * 0.5f; 
            target_pos.x = start.x + (end.x - start.x) * smooth_t;
            target_pos.y = start.y + (end.y - start.y) * smooth_t;
            target_pos.z = gait_config.step_height_mm * std::sin(PI * t_swing); // simple sine for vertical lift
        }
        else // STANCE PHASE (On the ground)
        {
            float t_stance = (leg_phase - swing_ratio) / gait_config.duty_factor;
            
            Vec2f stance_dist = leg_vel_linear * stance_time_s;
            
            target_pos = leg_default_pos[i];
            target_pos.x += stance_dist.x * (0.5f - t_stance); 
            target_pos.y += stance_dist.y * (0.5f - t_stance);
            target_pos.z = -gait_config.stance_depth_mm;
        }

        // send feet position
        body.setFeetPosition((Body::LegIndex)i, target_pos);
    }

    return Error::None;
}
