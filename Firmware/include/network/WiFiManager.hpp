#pragma once
#include "common/utils.hpp"
#include "common/NVS.hpp"
#include "network/DNSServer.hpp"

// ESP-IDF WiFi includes
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

class WiFiManager
{
public:
    constexpr static const char* TAG = "WiFiManager";

    enum State
    {
        Disconnected,
        Connecting,
        Connected
    };

    enum Mode
    {
        Station,
        AccessPoint,
    };

    WiFiManager();

    /**
     * @brief Initialize the WiFi manager.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the WiFi manager.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    /**
     * @brief Connect to a WiFi network.
     * @param ssid The SSID of the WiFi network.
     * @param password The password of the WiFi network.
     * @return Error code indicating success or failure.
     */
    Status connect(const char* _ssid = "", const char* _password = "", bool store_credentials = true);

    /**
     * @brief Disconnect from the current WiFi network.
     * @return Error code indicating success or failure.
     */
    Status disconnect();

    /**
     * @brief Scan for available WiFi networks.
     * @return Error code indicating success or failure.
     */
    Status createAccessPoint(const char* _ssid = "", const char* _password = "");

    /**
     * @brief Get the current WiFi state.
     * @return Current WiFi state.
     */
    State getState() const { return state; }

    /**
     * @brief Get the current WiFi mode.
     * @return Current WiFi mode.
     */
    Mode getMode() const { return mode; }

    const char* getIPAddr() const { return ip_address; }

    const char* getSSID() { return ssid; }

    void __wifi_event_handler(esp_event_base_t event_base, int32_t event_id, void* event_data);

private:
    State state;
    Mode mode;
    char mac_address[18];
    char ip_address[16];
    int retry_count;

    char ssid[32];
    char password[64];
    bool should_store_credentials;

    NVS::Handle* nvs_handle_ptr;
    esp_netif_t* sta_netif = nullptr;
    esp_netif_t* ap_netif = nullptr;

    Status __connect_to_ap();
    Status __create_ap();

    DNSServer dns_server;

    void on_ap_started();
    void on_ap_stopped();
    void on_ap_connected();
    void on_ap_disconnected();
};