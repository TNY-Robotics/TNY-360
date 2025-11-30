#pragma once
#include "modules/Menus.hpp"

typedef struct FaceEyesInfo
{
    float look_x;
    float look_y;
    float skew;
    float size;
    float open_left;
    float open_right;
    float lid_in_left;
    float lid_in_right;
    float lid_out_left;
    float lid_out_right;
    float lid_bottom_left;
    float lid_bottom_right;
} FaceEyesInfo;

using BehaviorFunction = void(*)(FaceEyesInfo& eyes_info, uint32_t time_ms);

class FaceMenu : public Menus::Menu
{
public:
    FaceEyesInfo eyes_info;

    FaceMenu();
    virtual ~FaceMenu();

    void setMainMenu(Menu* main_menu);
    void setBehavior(BehaviorFunction behavior);

protected:
    virtual void onBack() override;
    virtual void onSelect() override;
    virtual void onNext() override;
    virtual void onPrev() override;

    virtual void onCreate() override;
    virtual void onRender() override;
    virtual void onUpdate() override;

private:
    BehaviorFunction m_behavior = nullptr;
};

void Behavior_Idle(FaceEyesInfo& eyes_info, uint32_t time_ms);
