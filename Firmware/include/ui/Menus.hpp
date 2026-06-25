#pragma once
#include "common/utils.hpp"

namespace Menus
{
    /// MENU CLASS

    class Menu
    {
    public:
        constexpr static uint8_t HEADER_HEIGHT = 10;
        constexpr static uint8_t HEADER_PADDING = 5;
        constexpr static uint8_t HEADER_TITLE_ANIMATION_SHIFT = 16;

        Menu() = default;
        Menu(const char* title);
        Menu(const char* title, const uint8_t icon[8]);
        Menu(const char* title, Menu* parent);
        Menu(const char* title, Menu* parent, const uint8_t icon[8]);
        virtual ~Menu() = default;

        /// @brief Call action for left button short-press
        void onLeftPressed();
        /// @brief Call action for left button long-press
        void onLeftLongPressed();
        /// @brief Call action for right button short-press
        void onRightPressed();
        /// @brief Call action for right button long-press
        void onRightLongPressed();
        /// @brief Call action for when this menu starts being the current one
        void show();
        /// @brief Call action for when this menu stops being the current one
        void hide();
        /// @brief Call action for when this menu should be updated
        void update();
        /// @brief Call action for when this menu should be rendered. @note render flag check is already made inside the function.
        void render();

        const char* getTitle();
        const uint8_t* getIcon();

    protected:
        uint8_t m_icon[8] = {0};
        const char* m_title = "";
        Menu* parent = nullptr;

        /**
         * @brief Triggers render fonction for the next loop.
         * @note If this function isn't called during onUpdate(), the onRender() function will not be called.
         */
        void triggerRender();

        /// @brief Renders the menu's standard header (icon at top-left and title at top-center) 
        void renderHeader();

        /**
         * @brief "on back" action callback. This function is called when the left button is long-pressed.
         * @return Boolean true if action was handled. Return false to trigger default behavior.
         * @note Default behavior : Set current menu to parent if exists.
         */
        virtual bool onBack() = 0;
        /**
         * @brief "on select" action callback. This function is called when the right button is long-pressed.
         * @return Boolean true if action was handled. Return false to trigger default behavior.
         * @note Default behavior : None.
         */
        virtual bool onSelect() = 0;
        /**
         * @brief "on prev" action callback. This function is called when the left button is short-pressed.
         * @return Boolean true if action was handled. Return false to trigger default behavior.
         * @note Default behavior : None.
         */
        virtual bool onPrev() = 0;
        /**
         * @brief "on next" action callback. This function is called when the right button is short-pressed.
         * @return Boolean true if action was handled. Return false to trigger default behavior.
         * @note Default behavior : None.
         */
        virtual bool onNext() = 0;

        /// @brief "on show" action callback. This function is called when this menu starts being the current one.
        virtual void onShow() = 0;
        /// @brief "on hide" action callback. This function is called when this menu stops being the current one.
        virtual void onHide() = 0;
        /// @brief "on update" action callback. This function is called at each menu/screen cycle for state update (see SCREEN_REFRESH_RATE)
        virtual void onUpdate() = 0;
        /// @brief "on render" action callback. This function is called after onUpdate() if triggerRender() function was called.
        virtual void onRender() = 0;

    private:
        bool m_need_render = true;
        int8_t m_title_shift = 0;
    };

    /// MENUS NAMESPACE FUNCTIONS

    Status Init();

    Menu* GetCurrentMenu();

    void SetCurrentMenu(Menu* menu);

    // void DisplayError(ErrorStruct::ErrorStruct err);

    /// MENU INSTANCES

    Menu* GetMenuSplash();
    Menu* GetMenuFace();
    Menu* GetMenuError();
}