#include "locomotion/GaitPlanner.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"
#include <cmath>

GaitPlanner::GaitPlanner()
{
    // set default leg positions (relative to ground)
    leg_default_pos[0] = Vec3f( WALK_LEG_SPREAD_X_M,  WALK_LEG_SPREAD_Y_M, 0.f); // FL
    leg_default_pos[1] = Vec3f(-WALK_LEG_SPREAD_X_M,  WALK_LEG_SPREAD_Y_M, 0.f); // BL
    leg_default_pos[2] = Vec3f(-WALK_LEG_SPREAD_X_M, -WALK_LEG_SPREAD_Y_M, 0.f); // BR
    leg_default_pos[3] = Vec3f( WALK_LEG_SPREAD_X_M, -WALK_LEG_SPREAD_Y_M, 0.f); // FR

    setGaitConfig(GaitConfig()); // default config
}

GaitPlanner::GaitPlanner(GaitConfig config) : gait_config(config)
{
    // little trick to use the default constructor
    *this = GaitPlanner(); 
    setGaitConfig(config);
}

void GaitPlanner::setVelocityCommand(float x_ms, float y_ms, float z_rads)
{
    // Store the velocity command for use in the update loop
    cmd_vel_linear = Vec2f(x_ms, y_ms);
    cmd_vel_angular = z_rads;
}

void GaitPlanner::setGaitConfig(const GaitConfig& config)
{
    gait_config = config;
    apply_gait_offsets();
}

Status GaitPlanner::update(float dt, BodyCartesianState& state)
{
    if (cmd_vel_linear.x == 0 && cmd_vel_linear.y == 0 && cmd_vel_angular == 0)
    {
        if (is_moving) // just stopped, reset legs
        {
            is_moving = false;
            LOG_DEBUG(TAG, "Just stopped. Resetting legs");
        }

        for (int i = 0; i < 4; i++) {
            state.legs[i].target_pos = leg_default_pos[i];
            state.legs[i].phase = 0.0f;
            state.legs[i].is_swinging = false;
        }

        return Status::Ok;
    }
    if (!is_moving) // just started moving, reset gait phase
    {
        main_gait_phase = 0.0f;
        is_moving = true;
        LOG_DEBUG(TAG, "Started moving.");
    }

    // Advance the main gait phase
    main_gait_phase += (dt * gait_config.step_freq_hz);
    if (main_gait_phase >= 1.0f) main_gait_phase -= 1.0f;
    
    // Duration of phases in seconds (for distance calculations)
    float cycle_time_s = 1.0f / gait_config.step_freq_hz;
    float stance_time_s = cycle_time_s * gait_config.duty_factor;
    float swing_ratio = 1.0f - gait_config.duty_factor;

    for (int i = 0; i < 4; i++) {
        // local leg phase
        float leg_phase = main_gait_phase + leg_phase_offsets[i];
        if (leg_phase >= 1.0f) leg_phase -= 1.0f;

        // store gait infos in state
        state.legs[i].phase = leg_phase;
        state.legs[i].is_swinging = (leg_phase < swing_ratio);

        // I KNOW THIS ISNT MATHEMATICALLY RIGHT
        // But it saves the computation feet rotations,
        // and introduces an error way smaller than the mecanical backlash and all, so ...
        Vec2f leg_vel_linear = cmd_vel_linear;
        leg_vel_linear.x += -cmd_vel_angular * leg_default_pos[i].y;
        leg_vel_linear.y +=  cmd_vel_angular * leg_default_pos[i].x;

        if (state.legs[i].is_swinging) // --- SWING PHASE (In the air) ---
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
            float smooth_t = (1.0f - cosf(PI * t_swing)) * 0.5f;

            state.legs[i].target_pos.x = start.x + (end.x - start.x) * smooth_t;
            state.legs[i].target_pos.y = start.y + (end.y - start.y) * smooth_t;
            state.legs[i].target_pos.z = gait_config.step_height_m * sinf(PI * t_swing); // simple sine for vertical lift
        }
        else // STANCE PHASE (On the ground)
        {
            float t_stance = (leg_phase - swing_ratio) / gait_config.duty_factor;
            Vec2f stance_dist = leg_vel_linear * stance_time_s;
            
            state.legs[i].target_pos = leg_default_pos[i];
            state.legs[i].target_pos.x += stance_dist.x * (0.5f - t_stance); 
            state.legs[i].target_pos.y += stance_dist.y * (0.5f - t_stance);
            state.legs[i].target_pos.z = -gait_config.stance_depth_m;
        }
    }

    return Status::Ok;
}

void GaitPlanner::apply_gait_offsets()
{
    // Change la relation entre les pattes en fonction de l'allure choisie
    switch (gait_config.gait_type)
    {
        case GaitType::Walk: // Basic walk, Front Left with Back Right
            leg_phase_offsets[0] = 0.0f; // FL
            leg_phase_offsets[1] = 0.5f; // BL
            leg_phase_offsets[2] = 0.0f; // BR
            leg_phase_offsets[3] = 0.5f; // FR
            break;
        case GaitType::Creep: // Slow walking (note : duty factor should be > 0.75)
            leg_phase_offsets[0] = 0.00f; // FL
            leg_phase_offsets[1] = 0.50f; // BL
            leg_phase_offsets[2] = 0.25f; // BR
            leg_phase_offsets[3] = 0.75f; // FR
            if (gait_config.duty_factor < 0.75f)
                LOG_WARNING(TAG, "GaitType::Creep selected but gait_config.duty_factor is < 0.75f");
            break;
        case GaitType::Run: // Run, front legs together, back legs together
            leg_phase_offsets[0] = 0.0f; // FL
            leg_phase_offsets[1] = 0.5f; // BL
            leg_phase_offsets[2] = 0.5f; // BR
            leg_phase_offsets[3] = 0.0f; // FR
            break;
        default: // default to x pattern (walk)
            leg_phase_offsets[0] = 0.0f; leg_phase_offsets[1] = 0.5f;
            leg_phase_offsets[2] = 0.0f; leg_phase_offsets[3] = 0.5f;
            break;
    }
}