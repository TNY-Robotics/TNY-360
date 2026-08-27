#pragma once
#include "common/utils.hpp"
#include "common/NVS.hpp"
#include "common/config.hpp"
#include "network/DNSServer.hpp"

// ESP-IDF WiFi includes
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

/** <API_REF>
 * @type APAuthMode
 * @desc The authentication mode used by a Wi-Fi access point.
 */
enum class APAuthMode : uint8_t
{
    OPEN = 0,
    WEP,
    WPA_PSK,
    WPA2_PSK,
    WPA_WPA2_PSK,
    ENTERPRISE,
    WPA2_ENTERPRISE = ENTERPRISE,
    WPA3_PSK,
    WPA2_WPA3_PSK,
    WAPI_PSK,
    OWE,
    WPA3_ENT_192,
    WPA3_EXT_PSK,
    WPA3_EXT_PSK_MIXED_MODE,
    DPP,
    WPA3_ENTERPRISE,
    WPA2_WPA3_ENTERPRISE,
    MAX
};

/** <API_REF>
 * @type APDescription
 * @desc A description of a Wi-Fi access point.
 * @field ssid char[32] The SSID of the access point.
 * @field rssi int8_t The signal strength of the access point.
 * @field authmode APAuthMode The authentication mode used by the access point.
 */
struct APDescription
{
    char ssid[32];
    int8_t rssi;
    APAuthMode authmode;
};

class WiFiManager
{
public:
    constexpr static const char* TAG = "WiFiManager";

    using OnAPScanResultCallback = void (*)(const APDescription* ap_list, size_t ap_count, void* user_data);

    /// @brief Current state of the WiFiManager.
    enum State
    {
        /// @brief The WiFiManager is initializing. Will transition to Initialized or Error state after initialization.
        Initializing,
        /// @brief The WiFiManager is ready to be used, no STA or AP configured.
        Initialized,
        /// @brief The WiFiManager is in an error state.
        Error,
        /// @brief The WiFiManager is attempting to connect to an AP. Will transition to Connected or ConnectFailed state after attempting to connect.
        Connecting,
        /// @brief The WiFiManager is connected to an AP.
        Connected,
        /// @brief The WiFiManager failed to connect to an AP.
        ConnectFailed,
        /// @brief The WiFiManager is disconnecting from an AP. Will transition to Initialized state after disconnecting.
        Disconnecting,
        /// @brief The WiFiManager is starting an AP. Will transition to Started or StartFailed state after attempting to start.
        Starting,
        /// @brief The WiFiManager has started an AP.
        Started,
        /// @brief The WiFiManager failed to start an AP.
        StartFailed,
        /// @brief The WiFiManager is stopping an AP. Will transition to Initialized state after stopping.
        Stopping,
    };

    /// @brief Current mode of the WiFiManager, deduced from the current state.
    enum Mode
    {
        /// @brief The WiFiManager is in Station mode. connecting, connected, or disconnecting from an AP (or in failure state).
        STA,
        /// @brief The WiFiManager is in Access Point mode. Starting, started, or stopping his AP (or in failure state).
        AP,
        /// @brief The WiFiManager is in an unknown mode, in case of error or uninitialized state.
        None
    };

    /// @brief Structure to hold stored Wi-Fi credentials.
    struct WiFiCredentials
    {
        union {
            /// @brief Indicates whether valid credentials are stored.
            bool has_credentials;
            /// @brief Indicates whether the credentials should be stored in NVS.
            bool should_store_credentials;
        };
        /// @brief The stored SSID.
        char ssid[32];
        /// @brief The stored password.
        char password[64];
    };

    static const char* StateToString(State state)
    {
        switch (state)
        {
            case State::Initializing: return "Initializing";
            case State::Initialized: return "Initialized";
            case State::Error: return "Error";
            case State::Connecting: return "Connecting";
            case State::Connected: return "Connected";
            case State::ConnectFailed: return "ConnectFailed";
            case State::Disconnecting: return "Disconnecting";
            case State::Starting: return "Starting";
            case State::Started: return "Started";
            case State::StartFailed: return "StartFailed";
            case State::Stopping: return "Stopping";
            default: return "Unknown";
        }
    }

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

    /// STA MODE FUNCTIONS ///

    Status connectToAP(const char* ssid = nullptr, const char* password = nullptr, bool store_credentials = true, bool fallback_to_ap = true);

    Status disconnectFromAP();

    Status hasStoredAPCredentials(bool &has_credentials);

    Status forgetStoredAPCredentials();

    /// AP MODE FUNCTIONS ///

    Status startAP(const char* ssid = WIFI_AP_SSID, const char* password = WIFI_AP_PASSWORD);

    Status stopAP();

    /// GETTERS ///

    State getState() const { return state; }

    Mode getMode() const;

    const char* getSSID() const { return temp_credentials.ssid; }

    const char* getIPAddr() const { return ip_addr; }

    const char* getMACAddr() const { return mac_addr; }

    /// OTHER ///
    
    Status scanAP(OnAPScanResultCallback callback, void* user_data = nullptr);

    /// INTERNAL ///

    void __wifi_event_handler(esp_event_base_t event_base, int32_t event_id, void* event_data);

private:
    State state;
    char mac_addr[18];
    char ip_addr[16];
    uint8_t nb_retry_connect;

    bool should_fallback_to_ap;
    WiFiCredentials stored_credentials;
    WiFiCredentials temp_credentials;

    NVS::Handle* nvs_handle_ptr;
    esp_netif_t* sta_netif = nullptr;
    esp_netif_t* ap_netif = nullptr;

    OnAPScanResultCallback scan_callback = nullptr;
    void* scan_callback_user_data = nullptr;

    bool __in_transition_state();

    Status __connect_to_ap(const char* _ssid, const char* _password, bool store_credentials, bool fallback_to_ap);
    Status __disconnect_from_ap();
    Status __start_ap(const char* _ssid, const char* _password);
    Status __stop_ap();

    Status __load_stored_credentials();
    Status __save_credentials();
    Status __erase_stored_credentials();

    void on_ap_connecting();
    void on_ap_connected();
    void on_ap_disconnecting();
    void on_ap_disconnected();
    void on_ap_failed_connect();
    void on_ap_starting();
    void on_ap_started();
    void on_ap_stopping();
    void on_ap_stopped();
};