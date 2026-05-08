#include "ui/Menus.hpp"
#include "ui/Draw.hpp"
#include "ui/Button.hpp"
#include "ui/menus/Splash.hpp"
#include "ui/menus/Face.hpp"
#include "ui/menus/Error.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cmath>

namespace Menus
{
    /// MENU INSTANCES
    
    MenuSplash menuSplash;
    MenuFace menuFace;
    MenuError menuError;

    /// MENU CLASS FUNCTIONS

    Menu::Menu(const char* title)
        : m_title(title)
    {
    }

    Menu::Menu(const char* title, const uint8_t icon[8])
        : m_title(title)
    {
        memcpy(m_icon, icon, 8);
    }

    Menu::Menu(const char* title, Menu* parent)
        : m_title(title), parent(parent)
    {
    }

    Menu::Menu(const char* title, Menu* parent, const uint8_t icon[8])
        : m_title(title), parent(parent)
    {
        memcpy(m_icon, icon, 8);
    }

    void Menu::onLeftPressed()
    {
        if (onPrev()) return;
    }

    void Menu::onLeftLongPressed()
    {
        if (onBack()) return;
        if (parent)
        {
            Menus::SetCurrentMenu(parent);
        }
    }

    void Menu::onRightPressed()
    {
        if (onNext()) return;
    }

    void Menu::onRightLongPressed()
    {
        if (onSelect()) return;
    }

    void Menu::show()
    {
        if (m_title_shift == 0)
            m_title_shift = HEADER_TITLE_ANIMATION_SHIFT;

        onShow();
        m_need_render = true;
    }

    void Menu::hide()
    {
        if (m_title_shift == 0)
            m_title_shift = -HEADER_TITLE_ANIMATION_SHIFT;

        onHide();
        m_need_render = true;
    }

    void Menu::render()
    {
        if (m_title_shift == 1 || m_title_shift == -1) m_title_shift = 0;
        else if (m_title_shift > 0 || m_title_shift < 0)
        {
            m_title_shift -= m_title_shift / 2;
            m_need_render = true;
        }

        if (m_need_render)
        {
            ScreenDriver::Clear();
            onRender();
            ScreenDriver::Upload();
            m_need_render = false;
        }
    }

    void Menu::update()
    {
        onUpdate();
    }

    const char* Menu::getTitle()
    {
        return m_title;
    }

    const uint8_t* Menu::getIcon()
    {
        return m_icon;
    }

    void Menu::triggerRender()
    {
        m_need_render = true;
    }

    void Menu::renderHeader()
    {
        uint8_t text_width = Draw::GetTextWidth(m_title);
        Draw::RectFilled<false>(0, 0, ScreenDriver::info.width, Menu::HEADER_HEIGHT, ScreenDriver::COLOR_BLACK);
        Draw::Text<true>(m_title_shift - text_width / 2 + ScreenDriver::info.width / 2, 0, m_title);
        // Draw::Hline<false>(0, Menu::HEADER_HEIGHT, ScreenDriver::info.width);
        Draw::Blit<false>(0, 0, 8, 8, (uint8_t*)m_icon);
    }

    /// MENU NAMESPACE FUNCTIONS / VARIABLES

    bool btn_long_pressed[2] = {0, 0}; // just not to activate release if long pressed
    Menu* currentMenu = nullptr;

    void update_task(void* pvParams)
    {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t xFrequency = pdMS_TO_TICKS(SCREEN_REFRESH_RATE);

        while (true)
        {
            if (currentMenu)
            {
                currentMenu->update();
                currentMenu->render();
            }
            xTaskDelayUntil(&xLastWakeTime, xFrequency);
        }
    }

    Error Init()
    {
        // setup button callbacks
        if (Button::Init() != Error::None)
        {
            return Error::Unknown;
        }

        Button::SetCallbacks({
            .onPressed = {
                []() { Menus::btn_long_pressed[0] = false; },
                []() { Menus::btn_long_pressed[1] = false; }
            },
            .onReleased = {
                []() { if (!Menus::btn_long_pressed[0]) GetCurrentMenu()->onLeftPressed(); },
                []() { if (!Menus::btn_long_pressed[1]) GetCurrentMenu()->onRightPressed(); }
            },
            .onLongPressed = {
                []() { Menus::btn_long_pressed[0] = true; GetCurrentMenu()->onLeftLongPressed(); },
                []() { Menus::btn_long_pressed[1] = true; GetCurrentMenu()->onRightLongPressed(); }
            }
        });

        // Start update loop
        TaskHandle_t xTaskHandle = nullptr;
        if (xTaskCreate(update_task, "updateMenu", 8192, nullptr, 1, &xTaskHandle) != pdPASS)
        {
            return Error::Unknown;
        }

        return Error::None;
    }

    Menu* GetCurrentMenu()
    {
        return currentMenu;
    }

    void SetCurrentMenu(Menu* menu)
    {
        if (currentMenu) currentMenu->hide();
        currentMenu = menu;
        if (currentMenu) currentMenu->show();
    }

    void DisplayError(ErrorStruct::ErrorStruct err)
    {
        menuError.setError(err);
        SetCurrentMenu(&menuError);
    }

    Menu* GetMenuSplash() { return &menuSplash; }
    Menu* GetMenuFace() { return &menuFace; }
    Menu* GetMenuError() { return &menuError; }
}