#include "modules/WiFi.hpp"
#include "modules/Log.hpp"
#include "modules/NVS.hpp"
#include "config.hpp"
#include <cstring>
#include <cstdio>

// ESP-IDF WiFi includes
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

namespace WiFi
{
    NVS::Handle* nvs_handle = nullptr;

    bool initialized = false;
    bool connected = false;
    bool ap_started = false;
    char ip_address[16] = {0};
    char mac_address[18] = {0};
    esp_netif_t* sta_netif = nullptr;
    esp_netif_t* ap_netif = nullptr;

    WiFiCallbackThen connect_cb_then = nullptr;
    WiFiCallbackCatch connect_cb_catch = nullptr;
    WiFiCallbackThen ap_cb_then = nullptr;
    WiFiCallbackCatch ap_cb_catch = nullptr;
    const char* temp_ssid = nullptr;
    const char* temp_password = nullptr;
    uint8_t retry_count = 0;

    // Event handler for WiFi events
    void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
    {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        {
            Log::Add(Log::Level::Debug, "STA started, attempting to connect");
            esp_wifi_connect();
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            connected = false;
            memset(ip_address, 0, sizeof(ip_address));
            Log::Add(Log::Level::Debug, "WiFi disconnected");

            if (retry_count < WIFI_MAX_RETRIES)
            {
                esp_wifi_connect();
                retry_count++;
                Log::Add(Log::Level::Debug, "Reconnecting to WiFi (attempt %d)", retry_count);
            }
            else
            {
                Log::Add(Log::Level::Debug, "Failed to connect to WiFi after %d attempts", WIFI_MAX_RETRIES);
                if (connect_cb_catch) {
                    connect_cb_catch(Error::Unknown);
                    connect_cb_catch = nullptr;
                    connect_cb_then = nullptr;
                }
            }
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
            connected = true;
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            snprintf(ip_address, sizeof(ip_address), IPSTR, IP2STR(&event->ip_info.ip));
            Log::Add(Log::Level::Debug, "WiFi connected, IP obtained (%s)", ip_address);

            // store the connected SSID and password in NVS
            if (nvs_handle) {
                if (temp_ssid) // only store if we have a new SSID
                {
                    Log::Add(Log::Level::Debug, "Storing new WiFi credentials");
                    if (nvs_handle->set("ssid", temp_ssid, strlen(temp_ssid) + 1) != Error::Ok) {
                        Log::Add(Log::Level::Error, "Failed to store SSID in NVS");
                    }
                    else Log::Add(Log::Level::Debug, "SSID stored in NVS");

                    if (temp_password)
                    {
                        if (nvs_handle->set("password", temp_password, strlen(temp_password) + 1) != Error::Ok) {
                            Log::Add(Log::Level::Error, "Failed to store password in NVS");
                        }
                        else Log::Add(Log::Level::Debug, "Password stored in NVS");
                    }
                    else nvs_handle->set("password", "", 1); // clear password if none
                    temp_ssid = nullptr;
                    temp_password = nullptr;
                    retry_count = 0;
                    Log::Add(Log::Level::Debug, "WiFi credentials stored in NVS");
                }
                else
                {
                    Log::Add(Log::Level::Debug, "No new SSID to store");
                }
            } else
            {
                Log::Add(Log::Level::Error, "NVS handle not available, cannot store credentials");
            }
            
            if (connect_cb_then) {
                connect_cb_then();
                connect_cb_then = nullptr;
                connect_cb_catch = nullptr;
            }
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START)
        {
            ap_started = true;
            Log::Add(Log::Level::Debug, "WiFi AP started");
            sprintf(ip_address, "192.168.4.1"); // Default AP IP
            if (ap_cb_then) {
                ap_cb_then();
                ap_cb_then = nullptr;
                ap_cb_catch = nullptr;
            }
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP)
        {
            ap_started = false;
            Log::Add(Log::Level::Debug, "WiFi AP stopped");
            if (ap_cb_catch) {
                ap_cb_catch(Error::Unknown);
                ap_cb_catch = nullptr;
                ap_cb_then = nullptr;
            }
        }
    }

    Error Init()
    {
        if (initialized) {
            return Error::Ok;
        }

        // Initialize NVS
        if (Error err = NVS::Open("wifi", &nvs_handle); err != Error::Ok) {
            Log::Add(Log::Level::Error, "NVS Open failed");
            return Error::Unknown;
        }

        // Initialize network interface
        if (esp_netif_init() != ESP_OK)
        {
            Log::Add(Log::Level::Error, "esp_netif_init failed");
            return Error::Unknown;
        }
        if (esp_event_loop_create_default() != ESP_OK)
        {
            Log::Add(Log::Level::Error, "esp_event_loop_create_default failed");
            return Error::Unknown;
        }

        // Create default WiFi station and AP interfaces
        sta_netif = esp_netif_create_default_wifi_sta();
        ap_netif = esp_netif_create_default_wifi_ap();

        // Initialize WiFi
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        if (esp_wifi_init(&cfg) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "WiFi init failed");
            return Error::Unknown;
        }

        // Register event handlers
        if (esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Failed to register WiFi event handler");
            return Error::Unknown;
        }
        if (esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, nullptr) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Failed to register IP event handler");
            return Error::Unknown;
        }

        initialized = true;
        Log::Add(Log::Level::Debug, "WiFi initialized");

        return Error::Ok;
    }

    void Connect(WiFiCallbackThen cb_then, WiFiCallbackCatch cb_catch)
    {
        if (!initialized) {
            Init();
        }

        // Retrieve stored SSID and password from NVS
        char ssid[WIFI_MAX_SSID_LEN] = {0};
        char password[WIFI_MAX_PASSWORD_LEN] = {0};
        size_t ssid_len = sizeof(ssid);
        size_t password_len = sizeof(password);

        if (nvs_handle)
        {
            if ((nvs_handle->get("ssid", ssid, &ssid_len) != Error::Ok) ||
                (nvs_handle->get("password", password, &password_len) != Error::Ok))
            {
                Log::Add(Log::Level::Error, "Failed to read SSID or password from NVS");
                if (cb_catch) cb_catch(Error::Unknown);
                return;
            }
        }
        else
        {
            Log::Add(Log::Level::Error, "NVS handle not available");
            if (cb_catch) cb_catch(Error::Unknown);
            return;
        }

        if (ssid_len == 0) {
            Log::Add(Log::Level::Error, "No stored SSID found");
            if (cb_catch) cb_catch(Error::NotFound);
            return;
        }

        Connect(ssid, (password_len > 0) ? password : nullptr, cb_then, cb_catch, false);
    }

    void Connect(const char* ssid, const char* password, WiFiCallbackThen cb_then, WiFiCallbackCatch cb_catch, bool save)
    {
        if (!initialized) {
            Init();
        }

        if (!ssid) {
            if (cb_catch) cb_catch(Error::InvalidParameters);
            return;
        }

        Log::Add(Log::Level::Debug, "WiFi Connect requested, SSID: %s | Password: %s", ssid, password);

        // Stop WiFi if already running
        esp_wifi_stop();

        // Configure WiFi
        wifi_config_t wifi_config = {};
        strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
        if (password) {
            strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
        }

        // Set WiFi mode to station and start
        if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "WiFi set mode STA failed");
            if (cb_catch) cb_catch(Error::Unknown);
        }
        if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "WiFi set config STA failed");
            if (cb_catch) cb_catch(Error::Unknown);
        }
        if (esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G) != ESP_OK) // limit to older protocols for stability
        {
            Log::Add(Log::Level::Error, "WiFi set protocol failed");
            if (cb_catch) cb_catch(Error::Unknown);
        }
        if (esp_wifi_start() != ESP_OK)
        {
            Log::Add(Log::Level::Error, "WiFi start failed");
            if (cb_catch) cb_catch(Error::Unknown);
        }
        if (esp_wifi_set_max_tx_power(78) != ESP_OK) // set max TX power to ensure strong signal
        {
            Log::Add(Log::Level::Error, "WiFi set max TX power failed");
            if (cb_catch) cb_catch(Error::Unknown);
        }

        // Get MAC address
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        snprintf(mac_address, sizeof(mac_address), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        Log::Add(Log::Level::Debug, "WiFi connecting to: %s", ssid);

        // Reset retry count
        retry_count = 0;

        // Store callbacks
        connect_cb_then = cb_then;
        connect_cb_catch = cb_catch;

        // Store SSID and password temporarily
        if (save)
        {
            temp_ssid = ssid;
            temp_password = password;
        }
        else
        {
            temp_ssid = nullptr;
            temp_password = nullptr;
        }
    }

    void Disconnect()
    {
        if (!initialized) {
            return;
        }

        retry_count = WIFI_MAX_RETRIES; // set to max to avoid reconnecting

        esp_wifi_disconnect();
        connected = false;
        memset(ip_address, 0, sizeof(ip_address));
        Log::Add(Log::Level::Debug, "WiFi disconnected");
    }

    bool IsConnected()
    {
        return connected;
    }

    const char* GetIP()
    {
        if (!connected || strlen(ip_address) == 0) {
            return nullptr;
        }
        return ip_address;
    }

    const char* GetMAC()
    {
        if (!initialized) {
            Init();
        }
        return mac_address;
    }

    void StartAP(const char* ssid, const char* password, WiFiCallbackThen cb_then, WiFiCallbackCatch cb_catch)
    {
        if (!initialized) {
            Init();
        }

        if (!ssid) {
            Log::Add(Log::Level::Error, "WiFi AP: SSID cannot be null");
            if (cb_catch) cb_catch(Error::InvalidParameters);
            return;
        }

        // Stop current WiFi mode
        esp_wifi_stop();

        // Configure Access Point
        wifi_config_t wifi_config = {};
        strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
        wifi_config.ap.ssid_len = strlen(ssid);
        wifi_config.ap.channel = 1;
        wifi_config.ap.max_connection = 4;
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;

        if (password && strlen(password) > 0) {
            strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password) - 1);
            wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
        }

        if (esp_wifi_set_mode(WIFI_MODE_AP) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "WiFi set mode AP failed");
            if (cb_catch) cb_catch(Error::Unknown);
            return;
        }
        if (esp_wifi_set_config(WIFI_IF_AP, &wifi_config) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "WiFi set config AP failed");
            if (cb_catch) cb_catch(Error::Unknown);
            return;
        }
        if (esp_wifi_start() != ESP_OK)
        {
            Log::Add(Log::Level::Error, "WiFi start AP failed");
            if (cb_catch) cb_catch(Error::Unknown);
            return;
        }
        
        Log::Add(Log::Level::Debug, "Starting AP: %s", ssid);

        ap_cb_then = cb_then;
        ap_cb_catch = cb_catch;
    }

    void StopAP()
    {
        if (!initialized || !ap_started) {
            return;
        }

        esp_wifi_stop();
        ap_started = false;
        Log::Add(Log::Level::Debug, "Stopping AP");
    }

    bool IsAPStarted()
    {
        return ap_started;
    }
}