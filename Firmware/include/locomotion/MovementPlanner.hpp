#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp" // Suppose Vec2f, Vec3f exist
#include "common/config.hpp"

struct GaitConfig {
    /** @brief Step frequency in Hz */
    float step_freq_hz = 2.f;
    /** @brief Duty factor (portion of the gait cycle spent in stance) */
    float duty_factor = 0.60f;
    /** @brief Step height in mm */
    float step_height_mm = 20.0f;
    /** @brief Distance pushed by feets into the ground in mm */
    float stance_depth_mm = 0.0f;
};

class MovementPlanner {
public:
    constexpr static const float WALK_LEG_SPREAD_X_MM = DEFAULT_FEET_SPREAD_X_MM;
    constexpr static const float WALK_LEG_SPREAD_Y_MM = DEFAULT_FEET_SPREAD_Y_MM;

    MovementPlanner();

    /**
     * @brief Update the movement planner, should be called in the control loop
     * @return Error code indicating success or failure of the update
     */
    Error update();

    /**
     * @brief Set the desired velocity command for the robot
     * @param v_x Linear velocity in the x direction (mm/s)
     * @param v_y Linear velocity in the y direction (mm/s)
     * @param omega Angular velocity around the vertical axis (rad/s)
     */
    void setVelocityCommand(float v_x_mm_s, float v_y_mm_s, float omega_rad_s);

    /**
     * @brief Set the gait configuration parameters
     * @param config GaitConfig struct containing the desired gait parameters
     */
    void setGaitConfig(const GaitConfig& config) { gait_config = config; }

private:
    bool is_moving = false;

    GaitConfig gait_config;

    float main_gait_phase = 0.0f;

    Vec2f cmd_vel_linear; // mm/s
    float cmd_vel_angular; // rad/s

    Vec3f leg_default_pos[4]; // neutral position of each leg relative to the body center
};