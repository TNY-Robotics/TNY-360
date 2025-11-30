#pragma once
#include "utils.hpp"

namespace Menus
{
    class Menu
    {
    public:
        static const uint8_t HEADER_HEIGHT;
        static const uint8_t HEADER_PADDING;
        static const uint8_t HEADER_TITLE_ANIMATION_SHIFT;

        Menu() = default;
        Menu(const char* title);
        Menu(const char* title, const uint8_t icon[8]);
        Menu(const char* title, Menu* parent);
        Menu(const char* title, Menu* parent, const uint8_t icon[8]);
        virtual ~Menu() = default;

        void onLeftPressed();
        void onLeftLongPressed();
        void onRightPressed();
        void onRightLongPressed();
        void create();
        void render();
        void update();

        const char* getTitle();
        const uint8_t* getIcon();

    protected:
        uint8_t m_icon[8] = {0};
        int8_t m_title_shift = 0;
        const char* m_title = "";
        bool m_need_render = true;
        Menu* parent = nullptr;

        void renderHeader();

        virtual void onBack() = 0;
        virtual void onSelect() = 0;
        virtual void onNext() = 0;
        virtual void onPrev() = 0;

        virtual void onCreate() = 0;
        virtual void onRender() = 0;
        virtual void onUpdate() = 0;
    };

    Error Init();
    Menu* GetCurrentMenu();
    void SetCurrentMenu(Menu* menu);
}