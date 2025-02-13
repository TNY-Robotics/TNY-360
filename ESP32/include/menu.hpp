#pragma once
#include <stdint.h>
#include <esp_err.h>

class Menu
{
public:
    static const uint8_t HEADER_HEIGHT;
    static const uint8_t HEADER_PADDING;

    Menu() = default;
    Menu(const char* title);
    Menu(const char* title, Menu* parent);
    virtual ~Menu() = default;

    void onLeftPressed();
    void onLeftLongPressed();
    void onRightPressed();
    void onRightLongPressed();
    void render();
    void update();

    const char* getTitle();

protected:
    const char* m_title = "";
    bool m_need_render = true;
    Menu* parent = nullptr;

    void renderHeader();

    virtual void onBack() = 0;
    virtual void onSelect() = 0;
    virtual void onNext() = 0;
    virtual void onPrev() = 0;

    virtual void onRender() = 0;
    virtual void onUpdate() = 0;
};

esp_err_t menu_init();
Menu* get_current_menu();
void set_current_menu(Menu* menu);