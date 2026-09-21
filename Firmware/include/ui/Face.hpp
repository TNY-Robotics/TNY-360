#pragma once
#include "common/utils.hpp"

namespace Face
{
    //// DATA STRUCTURES ////

    /** <API_REF>
     * @type Face::EyesInfo
     * @desc A structure representing the state of the robot's eyes.
     * @field look_x float32 The x component of the look vector.
     * @field look_y float32 The y component of the look vector.
     * @field skew float32 The skew of the eyes.
     * @field size float32 The size multiplier for the eyes.
     * @field open_left float32 The open ratio of the left eye.
     * @field open_right float32 The open ratio of the right eye.
     * @field lid_in_left float32 The inner lid ratio of the left eye.
     * @field lid_in_right float32 The inner lid ratio of the right eye.
     * @field lid_out_left float32 The outer lid ratio of the left eye.
     * @field lid_out_right float32 The outer lid ratio of the right eye.
     * @field lid_bottom_left float32 The bottom lid ratio of the left eye.
     * @field lid_bottom_right float32 The bottom lid ratio of the right eye.
     */
    struct EyesInfo
    {
        // Eyes look X direction, range [-1.0 (right), 1.0 (left)]
        float look_x = 0.f;
        // Eyes look Y direction, range [-1.0 (down), 1.0 (up)]
        float look_y = 0.f;
        // Skew of the eyes, range [-1.0 (left above right), 1.0 (right above left)]
        float skew = 0.f;
        // Size multiplier for the eyes, 1.0 is default size
        float size = 1.f;
        // Left eye open ratio, 1.0 is fully open, 0.0 is fully closed
        float open_left = 1.2f;
        // Right eye open ratio, 1.0 is fully open, 0.0 is fully closed
        float open_right = 1.2f;
        // Left eye top inner lid ratio, 1.0 is fully covering the eye, 0.0 is not covering at all 
        float lid_in_left = 0.f;
        // Right eye top inner lid ratio, 1.0 is fully covering the eye, 0.0 is not covering at all
        float lid_in_right = 0.f;
        // Left eye top outer lid ratio, 1.0 is fully covering the eye, 0.0 is not covering at all
        float lid_out_left = 0.f;
        // Right eye top outer lid ratio, 1.0 is fully covering the eye, 0.0 is not covering at all
        float lid_out_right = 0.f;
        // Left eye bottom lid ratio, 1.0 is fully covering the eye, 0.0 is not covering at all
        float lid_bottom_left = 0.f;
        // Right eye bottom lid ratio, 1.0 is fully covering the eye, 0.0 is not covering at all
        float lid_bottom_right = 0.f;
    };
    
    //// BEHAVIOR FUNCTIONS ////

    using BehaviorFunction = void(*)(EyesInfo& eyes_info, uint32_t time_ms);

    // Blinks sometimes
    void Behavior_Blink(EyesInfo& eyes_info, uint32_t time_ms);
    // Looks around
    void Behavior_Idle(EyesInfo& eyes_info, uint32_t time_ms);
    // Controlled by API calls (user input)
    void Behavior_APIControl(EyesInfo& eyes_info, uint32_t time_ms);

    //// API CONTROL FUNCTIONS ////

    void setEyesInfos(const EyesInfo& eyes_info);

    // TODO : For now, Behavior_APIControl overrides everything, but we should be a flag system to allow for
    //        multiple layers of behaviors (blink, automatic look, etc.)
}