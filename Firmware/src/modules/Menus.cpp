#include "modules/Menus.hpp"
#include "modules/Screen.hpp"
#include "modules/menus/face.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace Menus
{
    const uint8_t Menu::HEADER_HEIGHT = 10;
    const uint8_t Menu::HEADER_PADDING = 5;

    Menu::Menu(const char* title)
        : m_title(title)
    {
    }

    Menu::Menu(const char* title, Menu* parent)
        : m_title(title), parent(parent)
    {
    }

    void Menu::onLeftPressed()
    {
        onPrev();
        m_need_render = true;
    }

    void Menu::onLeftLongPressed()
    {
        onBack();
        m_need_render = true;
    }

    void Menu::onRightPressed()
    {
        onNext();
        m_need_render = true;
    }

    void Menu::onRightLongPressed()
    {
        onSelect();
        m_need_render = true;
    }

    void Menu::render()
    {
        if (m_need_render)
        {
            onRender();
            m_need_render = false;
        }
    }

    void Menu::update()
    {
        onUpdate();
        if (m_need_render)
        {
            render();
        }
    }

    const char* Menu::getTitle()
    {
        return m_title;
    }

    void Menu::renderHeader()
    {
        // header
        uint16_t str_width = Draw::GetTextWidth(m_title);
        Draw::Text(&Screen::info, (Screen::info.width-str_width)/2, 0, m_title, Screen::COLOR_WHITE);
        Draw::Hline(&Screen::info, 0, Menu::HEADER_HEIGHT, Screen::info.width, Screen::COLOR_WHITE);
    }


    Menu* current_menu = nullptr;

    FaceMenu* face_menu = new FaceMenu();
    // MenuList* main_menu = new MenuList("Main Menu", face_menu);
    // MenuList* settings_menu = new MenuList("Settings", main_menu);
    // MenuList* network_menu = new MenuList("Network", main_menu);
    // MenuList* tests_menu = new MenuList("Tests", main_menu);
    // MenuList* about_menu = new MenuList("About", main_menu);
    // MenuList* bluetooth_menu = new MenuList("Bluetooth", network_menu);
    // MenuList* wifi_menu = new MenuList("Wi-Fi", network_menu);
    // MenuList* imu_menu = new MenuList("IMU", tests_menu);
    // WifiStatusMenu* wifi_status_menu = new WifiStatusMenu(wifi_menu);
    // WifiConnectMenu* wifi_connect_menu = new WifiConnectMenu(wifi_menu);
    // WifiDisconnectMenu* wifi_disconnect_menu = new WifiDisconnectMenu(wifi_menu);
    // GyroMenu* gyro_menu = new GyroMenu(imu_menu);
    // AcceMenu* acce_menu = new AcceMenu(imu_menu);
    // TempMenu* temp_menu = new TempMenu(imu_menu);
    // IMUCalibMenu* imu_calib_menu = new IMUCalibMenu(imu_menu);
    // PressureMenu* pressure_menu = new PressureMenu(tests_menu);
    // MotorMenu* motor_menu = new MotorMenu(tests_menu);
    // I2CMenu* i2c_menu = new I2CMenu(tests_menu);
    // PowerMenu* power_menu = new PowerMenu(tests_menu);

    // Menu* main_menu_childs[] = {
    //     settings_menu,
    //     network_menu,
    //     tests_menu,
    //     about_menu
    // };
    // Menu* settings_menu_childs[] = {
    //     new FactoryResetMenu(settings_menu)
    // };
    // Menu* network_menu_childs[] = {
    //     bluetooth_menu,
    //     wifi_menu
    // };
    // Menu* wifi_menu_childs[] = {
    //     wifi_status_menu,
    //     wifi_connect_menu,
    //     wifi_disconnect_menu
    // };
    // Menu* test_menu_childs[] = {
    //     imu_menu,
    //     pressure_menu,
    //     motor_menu,
    //     i2c_menu,
    //     power_menu
    // };
    // Menu* imu_menu_childs[] = {
    //     gyro_menu,
    //     acce_menu,
    //     temp_menu,
    //     imu_calib_menu
    // };

    void update_task(void* pvParams)
    {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t xFrequency = pdMS_TO_TICKS(33); // approx 30 FPS

        while (true)
        {
            if (current_menu)
            {
                current_menu->update();
            }
            xTaskDelayUntil(&xLastWakeTime, xFrequency);
        }
    }

    Error Init()
    {
        // face_menu->setMainMenu(main_menu);
        // main_menu->setItems(main_menu_childs, 4);
        // settings_menu->setItems(settings_menu_childs, 1);
        // network_menu->setItems(network_menu_childs, 2);
        // wifi_menu->setItems(wifi_menu_childs, 3);
        // tests_menu->setItems(test_menu_childs, 5);
        // imu_menu->setItems(imu_menu_childs, 4);
        SetCurrentMenu(face_menu);

        // Start update loop
        TaskHandle_t xTaskHandle = nullptr;
        if (xTaskCreate(update_task, "updateMenu", 8192, nullptr, 1, &xTaskHandle) != pdPASS)
        {
            return Error::Unknown;
        }

        return Error::Ok;
    }

    Menu* GetCurrentMenu()
    {
        return current_menu;
    }

    void SetCurrentMenu(Menu* menu)
    {
        current_menu = menu;
    }
}