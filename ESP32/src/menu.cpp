#include "menu.hpp"
#include "menulist.hpp"
#include "menus/factory_reset.hpp"
#include "menus/eyes.hpp"
#include "menus/wifi_status.hpp"
#include "menus/wifi_connect.hpp"
#include "menus/wifi_disconnect.hpp"
#include "screen.hpp"
#include "draw.hpp"

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
}

const char* Menu::getTitle()
{
    return m_title;
}

void Menu::renderHeader()
{
    screen_info_t* screen_info = get_screen_info();
    
    // header
    uint16_t str_width = draw_text_width(m_title);
    draw_text(screen_info, (screen_info->width-str_width)/2, 0, m_title, COLOR_WHITE);
    draw_hline(screen_info, 0, Menu::HEADER_HEIGHT, screen_info->width, COLOR_WHITE);
}


Menu* current_menu = nullptr;

EyesMenu* eyes_menu = new EyesMenu();
MenuList* main_menu = new MenuList("Main Menu", eyes_menu);
MenuList* settings_menu = new MenuList("Settings", main_menu);
MenuList* network_menu = new MenuList("Network", main_menu);
MenuList* tests_menu = new MenuList("Tests", main_menu);
MenuList* about_menu = new MenuList("About", main_menu);
MenuList* bluetooth_menu = new MenuList("Bluetooth", network_menu);
MenuList* wifi_menu = new MenuList("Wi-Fi", network_menu);
WifiStatusMenu* wifi_status_menu = new WifiStatusMenu(wifi_menu);
WifiConnectMenu* wifi_connect_menu = new WifiConnectMenu(wifi_menu);
WifiDisconnectMenu* wifi_disconnect_menu = new WifiDisconnectMenu(wifi_menu);


Menu* main_menu_childs[] = {
    settings_menu,
    network_menu,
    tests_menu,
    about_menu
};
Menu* settings_menu_childs[] = {
    new FactoryResetMenu(settings_menu)
};
Menu* network_menu_childs[] = {
    bluetooth_menu,
    wifi_menu
};
Menu* wifi_menu_childs[] = {
    wifi_status_menu,
    wifi_connect_menu,
    wifi_disconnect_menu
};

esp_err_t menu_init()
{
    eyes_menu->setMainMenu(main_menu);
    main_menu->setItems(main_menu_childs, 4);
    settings_menu->setItems(settings_menu_childs, 1);
    network_menu->setItems(network_menu_childs, 2);
    wifi_menu->setItems(wifi_menu_childs, 3);
    set_current_menu(eyes_menu);
    return ESP_OK;
}

Menu* get_current_menu()
{
    return current_menu;
}

void set_current_menu(Menu* menu)
{
    current_menu = menu;
}