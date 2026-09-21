#include "ui/menus/Face.hpp"
#include "ui/Draw.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
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

    // set default behavior to idle - Disabled, network control instead
    // setBehavior(Behavior_Idle);
    setBehavior(Face::Behavior_Idle);
}

MenuFace::~MenuFace()
{
}

void MenuFace::setBehavior(Face::BehaviorFunction behavior)
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

    Face::EyesInfo base_infos = eyes_info; // copy to avoid modifying the original infos
    if (m_behavior) // apply the behavior function
    {
        m_behavior(base_infos, xTaskGetTickCount() * portTICK_PERIOD_MS);
    }

    // Display the eyes based on eyes_info

    uint8_t m_eyes_size = EYES_SIZE * base_infos.size;
    float look_x_right = -(-base_infos.look_x*2 - base_infos.look_x*base_infos.look_x);
    float look_x_left = base_infos.look_x*2 - base_infos.look_x*base_infos.look_x;
    
    Draw::RectRounded( // right eye white
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        m_eyes_size,
        m_eyes_size * base_infos.open_right,
        7,
        Screen::COLOR_WHITE
    );
    if (eyes_info.lid_in_right > 0.001f) Draw::TriangleFilled( // right eyelid in
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f + (base_infos.lid_in_right * m_eyes_size * base_infos.open_right),
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_out_right > 0.001f) Draw::TriangleFilled( // right eyelid out
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f + (base_infos.lid_out_right * m_eyes_size * base_infos.open_right),
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_bottom_right > 0.001f) Draw::TriangleFilled( // right eyelid bottom
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width / 4  + look_x_right * 10.0f,
        Screen::GetInstance()->getInfo().height / 2 + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_right) / 2 - base_infos.skew * 5.0f - (base_infos.lid_bottom_right * m_eyes_size * base_infos.open_right),
        Screen::COLOR_BLACK
    );

    Draw::RectRounded( // left eye white
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        m_eyes_size,
        m_eyes_size * base_infos.open_left,
        7,
        Screen::COLOR_WHITE
    );
    if (eyes_info.lid_in_left > 0.001f) Draw::TriangleFilled( // left eyelid in
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f + (base_infos.lid_in_left * m_eyes_size * base_infos.open_left),
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_out_left > 0.001f) Draw::TriangleFilled( // left eyelid out
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f + (base_infos.lid_out_left * m_eyes_size * base_infos.open_left),
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f - (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::COLOR_BLACK
    );
    if (eyes_info.lid_bottom_left > 0.001f) Draw::TriangleFilled( // left eyelid bottom
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f + m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f - m_eyes_size / 2,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f,
        Screen::GetInstance()->getInfo().width * 3 / 4 + look_x_left * 10.0f,
        Screen::GetInstance()->getInfo().height / 2    + base_infos.look_y * 10.0f + (m_eyes_size * base_infos.open_left) / 2 + base_infos.skew * 5.0f - (base_infos.lid_bottom_left * m_eyes_size * base_infos.open_left),
        Screen::COLOR_BLACK
    );
}

void MenuFace::onUpdate()
{
    triggerRender(); // always need render to update eyes
}
