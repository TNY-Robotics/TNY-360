#include "ui/Face.hpp"
#include <esp_random.h>

namespace Face
{
    //// BEHAVIOR FUNCTIONS ////

    uint32_t last_blink_time = 0;
    uint32_t blink_wait_time = 0;
    float blink_duration = 150.f; // in ms
    bool is_blinking = false;
    uint32_t last_look_move = 0;
    uint32_t look_move_wait_time = 0;
    uint32_t last_smalllook_move = 0;
    uint32_t smalllook_move_wait_time = 0;
    float last_look_x_shift = 0.0f;
    float last_look_y_shift = 0.0f;
    float look_x_shift = 0.0f;
    float look_y_shift = 0.0f;
    float smalllook_x_shift = 0.0f;
    float smalllook_y_shift = 0.0f;
    float last_smalllook_x_shift = 0.0f;
    float last_smalllook_y_shift = 0.0f;

    void Behavior_Blink(EyesInfo& eyes_info, uint32_t time_ms)
    {
        // Blink every 3 to 7 seconds
        if (!is_blinking && (time_ms - last_blink_time) > blink_wait_time)
        {
            is_blinking = true;
            last_blink_time = time_ms;
            blink_wait_time = 3000 + (esp_random() % 4000); // next blink in 3-7s
        }

        if (is_blinking)
        {
            float blink_progress = float(time_ms - last_blink_time) / blink_duration;
            if (blink_progress >= 1.0f)
            {
                // End of blink
                eyes_info.open_left *= 1.0f;
                eyes_info.open_right *= 1.0f;
                is_blinking = false;
            }
            else
            {
                // Closing and opening the eyes
                if (blink_progress < 0.5f)
                {
                    // Closing
                    eyes_info.open_left *= 1.0f - (blink_progress * 2.0f);
                    eyes_info.open_right *= 1.0f - (blink_progress * 2.0f);
                }
                else
                {
                    // Opening
                    eyes_info.open_left *= (blink_progress - 0.5f) * 2.0f;
                    eyes_info.open_right *= (blink_progress - 0.5f) * 2.0f;
                }
            }
        }
    }

    void Behavior_Idle(EyesInfo& eyes_info, uint32_t time_ms)
    {
        Behavior_Blink(eyes_info, time_ms); // also apply blinking in idle behavior

        // Move eyes every 1 to 5 seconds
        if ((time_ms - last_look_move) > look_move_wait_time)
        {
            last_look_move = time_ms;
            look_move_wait_time = 2000 + (esp_random() % 4000); // next move in 2-6s

            // Random look position between -0.9 and 0.9
            look_x_shift = ((esp_random() % 1800) / 1000.0f) - 0.9f;
            look_y_shift = ((esp_random() % 1800) / 1000.0f) - 0.9f;
        }

        // Move eyes every 1 to 5 seconds
        if ((time_ms - last_smalllook_move) > smalllook_move_wait_time)
        {
            last_smalllook_move = time_ms;
            smalllook_move_wait_time = 300 + (esp_random() % 500); // next move in 300-800ms

            // Random look position between -0.1 and 0.1
            smalllook_x_shift = ((esp_random() % 200) / 1000.0f) - 0.1f;
            smalllook_y_shift = ((esp_random() % 200) / 1000.0f) - 0.1f;
        }

        last_look_x_shift += (look_x_shift - last_look_x_shift) * 0.1f;
        last_look_y_shift += (look_y_shift - last_look_y_shift) * 0.1f;

        last_smalllook_x_shift += (smalllook_x_shift - last_smalllook_x_shift) * 0.5f;
        last_smalllook_y_shift += (smalllook_y_shift - last_smalllook_y_shift) * 0.5f;

        eyes_info.look_x += last_look_x_shift + last_smalllook_x_shift;
        eyes_info.look_y += last_look_y_shift + last_smalllook_y_shift;
    }

    EyesInfo current_eyes_info;
    EyesInfo target_eyes_info;

    void Behavior_APIControl(EyesInfo& eyes_info, uint32_t time_ms)
    {
        // Morph current eyes info towards target eyes info
        float morph_speed = 0.5f;
        current_eyes_info.look_x += (target_eyes_info.look_x - current_eyes_info.look_x) * morph_speed;
        current_eyes_info.look_y += (target_eyes_info.look_y - current_eyes_info.look_y) * morph_speed;
        current_eyes_info.skew += (target_eyes_info.skew - current_eyes_info.skew) * morph_speed;
        current_eyes_info.size += (target_eyes_info.size - current_eyes_info.size) * morph_speed;
        current_eyes_info.open_left += (target_eyes_info.open_left - current_eyes_info.open_left) * morph_speed;
        current_eyes_info.open_right += (target_eyes_info.open_right - current_eyes_info.open_right) * morph_speed;
        current_eyes_info.lid_in_left += (target_eyes_info.lid_in_left - current_eyes_info.lid_in_left) * morph_speed;
        current_eyes_info.lid_in_right += (target_eyes_info.lid_in_right - current_eyes_info.lid_in_right) * morph_speed;
        current_eyes_info.lid_out_left += (target_eyes_info.lid_out_left - current_eyes_info.lid_out_left) * morph_speed;
        current_eyes_info.lid_out_right += (target_eyes_info.lid_out_right - current_eyes_info.lid_out_right) * morph_speed;
        current_eyes_info.lid_bottom_left += (target_eyes_info.lid_bottom_left - current_eyes_info.lid_bottom_left) * morph_speed;
        current_eyes_info.lid_bottom_right += (target_eyes_info.lid_bottom_right - current_eyes_info.lid_bottom_right) * morph_speed;

        // Update the output eyes info (complete override, fuck the behavior chain)
        eyes_info = current_eyes_info;
    }

    void setEyesInfos(const EyesInfo& eyes_info)
    {
        target_eyes_info = eyes_info;
    }
}