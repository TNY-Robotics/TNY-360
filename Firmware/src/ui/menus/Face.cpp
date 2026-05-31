#include "ui/menus/Face.hpp"
#include "ui/Draw.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <esp_random.h>
#include <cmath>

constexpr uint8_t EYES_SIZE = 30;

MenuFace* MenuFace::instance = nullptr;

MenuFace::MenuFace()
    : Menu("Face"), menuMain(MenuMain(this))
{
    instance = this;

    // set defaut eyes info (open looking forward)
    eyes_info.look_x = 0.0f;
    eyes_info.look_y = 0.0f;
    eyes_info.skew = 0.0f;
    eyes_info.size = 1.0f;
    eyes_info.open_left = 1.2f;
    eyes_info.open_right = 1.2f;
    eyes_info.lid_in_left = 0.0f;
    eyes_info.lid_in_right = 0.0f;
    eyes_info.lid_out_left = 0.0f;
    eyes_info.lid_out_right = 0.0f;
    eyes_info.lid_bottom_left = 0.0f;
    eyes_info.lid_bottom_right = 0.0f;

    // set default behavior to idle
    setBehavior(Behavior_Idle);
}

MenuFace::~MenuFace()
{
}

void MenuFace::setBehavior(BehaviorFunction behavior)
{
    m_behavior = behavior;
}

bool MenuFace::onBack()
{
    Menus::SetCurrentMenu(&menuMain);
    return true;
}

bool MenuFace::onSelect()
{
    Menus::SetCurrentMenu(&menuMain);
    return true;
}

bool MenuFace::onNext()
{
    return false;
}

bool MenuFace::onPrev()
{
    return false;
}

void MenuFace::onShow()
{
}

void MenuFace::onHide()
{
}

void MenuFace::onRender()
{
    // Note : doing update here not to use power if not rendering

    FaceEyesInfo base_infos = eyes_info; // copy to avoid modifying the original infos
    if (m_behavior) // apply the behavior function
    {
        m_behavior(base_infos, xTaskGetTickCount() * portTICK_PERIOD_MS);
    }

    // Display the eyes based on eyes_info

    uint8_t m_eyes_size = EYES_SIZE * base_infos.size;
    float look_x_right = -(-base_infos.look_x*2 - base_infos.look_x*base_infos.look_x);
    float look_x_left = base_infos.look_x*2 - base_infos.look_x*base_infos.look_x;
    
    Draw::RectRounded( // right eye white
        ScreenDriver::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        m_eyes_size,
        m_eyes_size * base_infos.open_right,
        7,
        ScreenDriver::COLOR_WHITE
    );
    if (eyes_info.lid_in_right > 0.001f) Draw::TriangleFilled( // right eyelid in
        ScreenDriver::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        ScreenDriver::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        ScreenDriver::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f + (base_infos.lid_in_right * m_eyes_size * base_infos.open_right),
        ScreenDriver::COLOR_BLACK
    );
    if (eyes_info.lid_out_right > 0.001f) Draw::TriangleFilled( // right eyelid out
        ScreenDriver::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        ScreenDriver::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f + (base_infos.lid_out_right * m_eyes_size * base_infos.open_right),
        ScreenDriver::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        ScreenDriver::COLOR_BLACK
    );
    if (eyes_info.lid_bottom_right > 0.001f) Draw::TriangleFilled( // right eyelid bottom
        ScreenDriver::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        ScreenDriver::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        ScreenDriver::info.width / 4  + look_x_right * 10.0f,
        ScreenDriver::info.height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f - (base_infos.lid_bottom_right * m_eyes_size * base_infos.open_right),
        ScreenDriver::COLOR_BLACK
    );

    Draw::RectRounded( // left eye white
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        m_eyes_size,
        m_eyes_size * base_infos.open_left,
        7,
        ScreenDriver::COLOR_WHITE
    );
    if (eyes_info.lid_in_left > 0.001f) Draw::TriangleFilled( // left eyelid in
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f + (base_infos.lid_in_left * m_eyes_size * base_infos.open_left),
        ScreenDriver::COLOR_BLACK
    );
    if (eyes_info.lid_out_left > 0.001f) Draw::TriangleFilled( // left eyelid out
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f + (base_infos.lid_out_left * m_eyes_size * base_infos.open_left),
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        ScreenDriver::COLOR_BLACK
    );
    if (eyes_info.lid_bottom_left > 0.001f) Draw::TriangleFilled( // left eyelid bottom
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        ScreenDriver::info.width * 3 / 4 + look_x_left * 10.0f,
        ScreenDriver::info.height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f - (base_infos.lid_bottom_left * m_eyes_size * base_infos.open_left),
        ScreenDriver::COLOR_BLACK
    );
}

void MenuFace::onUpdate()
{
    triggerRender(); // always need render to update eyes
}


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

void Behavior_Blink(FaceEyesInfo& eyes_info, uint32_t time_ms)
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

void Behavior_Idle(FaceEyesInfo& eyes_info, uint32_t time_ms)
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