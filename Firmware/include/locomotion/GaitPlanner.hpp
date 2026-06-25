#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp" // Suppose Vec2f, Vec3f exist
#include "common/config.hpp"
#include "locomotion/utils.hpp"

class GaitPlanner : public CartesianStateModifier
{
public:
    enum class GaitType {
        Creep,  // One leg at a time (safe) [note : duty_factor > 0.75f]
        Walk,   // Two legs at a time (X pattern)
        Run,    // Two legs at a time (front vs back)
        // Jump    // Four legs in the air
    };

    struct GaitConfig
    {
        /// @brief Step frequency in Hz
        float step_freq_hz = 2.f;
        /// @brief Duty factor (portion of the gait cycle spent in stance)
        float duty_factor = 0.60f;
        /// @brief Step height in meters
        float step_height_m = 0.02f;
        /// @brief Distance pushed by feets into the ground in meters
        float stance_depth_m = 0.0f;
        /// @brief Type of gait to base the movements of
        GaitType gait_type = GaitType::Walk;
    };

    constexpr static const char* TAG = "GaitPlanner";
    constexpr static const float WALK_LEG_SPREAD_X_M = DEFAULT_FEET_SPREAD_X_M;
    constexpr static const float WALK_LEG_SPREAD_Y_M = DEFAULT_FEET_SPREAD_Y_M;

    GaitPlanner();

    GaitPlanner(GaitConfig config);

    /**
     * @brief Update the movement planner, should be called in the control loop
     * @return Error code indicating success or failure of the update
     */
    Status update(float dt, BodyCartesianState& state) override;

    /**
     * @brief Set the desired velocity command for the robot
     * @param x_ms Linear velocity in the x direction (m/s)
     * @param y_ms Linear velocity in the y direction (m/s)
     * @param z_rads Angular velocity around the vertical axis (rad/s)
     */
    void setVelocityCommand(float x_ms, float y_ms, float z_rads);

    /**
     * @brief Set the gait configuration parameters
     * @param config GaitConfig struct containing the desired gait parameters
     */
    void setGaitConfig(const GaitConfig& config);

    /**
     * @brief Get the current gait configuration
     * @return GaitConfig object, non-modifiable (use setGaitConfig)
     */
    inline const GaitConfig& getConfig() const { return gait_config; }

private:
    bool is_moving = false;

    GaitConfig gait_config;

    float main_gait_phase = 0.0f;

    Vec2f cmd_vel_linear; // m/s
    float cmd_vel_angular; // rad/s

    Vec3f leg_default_pos[4]; // neutral position of each leg relative to the body center
    
    float leg_phase_offsets[4]; // gait cycle offset

    /// @brief Loads the leg_phase_offsets with the gait offsets matching the current GaitType
    void apply_gait_offsets();
};