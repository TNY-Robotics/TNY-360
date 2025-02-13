#pragma once
#include "menu.hpp"
#include "freertos/FreeRTOS.h"

class EyesMenu : public Menu
{
    static const uint8_t EYES_SIZE;

public:
    EyesMenu();
    virtual ~EyesMenu();

    void setMainMenu(Menu* main_menu);

protected:
    int8_t m_eyes_current_shift_x = 0;
    int8_t m_eyes_current_shift_y = 0;
    int8_t m_eyes_target_shift_x = 0;
    int8_t m_eyes_target_shift_y = 0;
    int8_t m_eyes_size = 10;
    TickType_t m_random_eyes_last_update = 0;
    TickType_t m_random_eyes_update_delay = 0;
    TickType_t m_random_blink_last_update = 0;
    TickType_t m_random_blink_update_delay = 0;

    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onRender() override;
    virtual void onUpdate() override;
};
