#include "modules/menus/face.hpp"
#include "modules/Screen.hpp"
#include "modules/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <esp_random.h>
#include <cmath>

constexpr uint8_t EYES_SIZE = 30;

FaceMenu::FaceMenu()
    : Menu()
{
    // set defaut eyes info (open looking forward)
    eyes_info.look_x = 0.0f;
    eyes_info.look_y = 0.0f;
    eyes_info.skew = 0.0f;
    eyes_info.size = 1.0f;
    eyes_info.open_left = 0.8f;
    eyes_info.open_right = 0.8f;
    eyes_info.lid_in_left = 0.0f;
    eyes_info.lid_in_right = 0.0f;
    eyes_info.lid_out_left = 0.0f;
    eyes_info.lid_out_right = 0.0f;
    eyes_info.lid_bottom_left = 0.0f;
    eyes_info.lid_bottom_right = 0.0f;

    // set default behavior to idle
    setBehavior(Behavior_Idle);
}

FaceMenu::~FaceMenu()
{
}

void FaceMenu::setMainMenu(Menu* main_menu)
{
    parent = main_menu; // note : using Menu::parent as pointer on main menu
}

void FaceMenu::setBehavior(BehaviorFunction behavior)
{
    m_behavior = behavior;
}

void FaceMenu::onBack()
{
    if (parent) // note : using Menu::parent as pointer on main menu
    {
        Menus::SetCurrentMenu(parent);
        m_need_render = true;
    }
}

void FaceMenu::onSelect()
{
    if (parent) // note : using Menu::parent as pointer on main menu
    {
        Menus::SetCurrentMenu(parent);
        m_need_render = true;
    }
}

void FaceMenu::onNext()
{
}

void FaceMenu::onPrev()
{
}

void FaceMenu::onCreate()
{
}

void FaceMenu::onRender()
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
    
    Screen::Clear();
    Draw::RectRounded( // right eye white
        &Screen::info,
        Screen::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        m_eyes_size,
        m_eyes_size * base_infos.open_right,
        7,
        Screen::COLOR_WHITE
    );
    if (eyes_info.lid_in_right > 0.001f) Draw::TriangleFilled( // right eyelid in
        &Screen::info,
        Screen::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f + (base_infos.lid_in_right * m_eyes_size * base_infos.open_right),
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_out_right > 0.001f) Draw::TriangleFilled( // right eyelid out
        &Screen::info,
        Screen::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f + (base_infos.lid_out_right * m_eyes_size * base_infos.open_right),
        Screen::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_bottom_right > 0.001f) Draw::TriangleFilled( // right eyelid bottom
        &Screen::info,
        Screen::info.width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::info.width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::info.width / 4  + look_x_right * 10.0f,
        Screen::info.height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f - (base_infos.lid_bottom_right * m_eyes_size * base_infos.open_right),
        Screen::COLOR_BLACK
    );

    Draw::RectRounded( // left eye white
        &Screen::info,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        m_eyes_size,
        m_eyes_size * base_infos.open_left,
        7,
        Screen::COLOR_WHITE
    );
    if (eyes_info.lid_in_left > 0.001f) Draw::TriangleFilled( // left eyelid in
        &Screen::info,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f + (base_infos.lid_in_left * m_eyes_size * base_infos.open_left),
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_out_left > 0.001f) Draw::TriangleFilled( // left eyelid out
        &Screen::info,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f + (base_infos.lid_out_left * m_eyes_size * base_infos.open_left),
        Screen::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_bottom_left > 0.001f) Draw::TriangleFilled( // left eyelid bottom
        &Screen::info,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::info.height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::info.width * 3 / 4 + look_x_left * 10.0f,
        Screen::info.height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f - (base_infos.lid_bottom_left * m_eyes_size * base_infos.open_left),
        Screen::COLOR_BLACK
    );

    Screen::Upload();
}

void FaceMenu::onUpdate()
{
    m_need_render = true; // always need render to update eyes
}


//// BEHAVIOR FUNCTIONS ////

uint32_t last_blink_time = 0;
uint32_t blink_wait_time = 0;
float blink_duration = 150.f; // in ms
bool is_blinking = false;
uint32_t last_look_move = 0;
uint32_t look_move_wait_time = 0;
float last_look_x_shift = 0.0f;
float last_look_y_shift = 0.0f;
float look_x_shift = 0.0f;
float look_y_shift = 0.0f;

void Behavior_Idle(FaceEyesInfo& eyes_info, uint32_t time_ms)
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

    // Move eyes every 1 to 5 seconds
    if ((time_ms - last_look_move) > look_move_wait_time)
    {
        last_look_move = time_ms;
        look_move_wait_time = 1000 + (esp_random() % 4000); // next move in 1-5s

        // Random look position between -1.0 and 1.0
        look_x_shift = ((esp_random() % 2000) / 1000.0f) - 1.0f;
        look_y_shift = ((esp_random() % 2000) / 1000.0f) - 1.0f;
    }

    last_look_x_shift += (look_x_shift - last_look_x_shift) * 0.1f;
    last_look_y_shift += (look_y_shift - last_look_y_shift) * 0.1f;

    eyes_info.look_x += last_look_x_shift;
    eyes_info.look_y += last_look_y_shift;
}