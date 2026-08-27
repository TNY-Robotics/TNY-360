#include "network/WiFiManager.hpp"
#include "common/Log.hpp"
#include "common/NVS.hpp"
#include "Robot.hpp"

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    WiFiManager* self = static_cast<WiFiManager*>(arg);
    self->__wifi_event_handler(event_base, event_id, event_data);
}

void WiFiManager::__wifi_event_handler(esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    // Disconnected from AP
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (state == Connecting)
        {
            LOG_DEBUG(TAG, "WiFi failed to connect to AP");
            state = ConnectFailed;
            on_ap_failed_connect();
        }
        else if (state == Disconnecting)
        {
            LOG_DEBUG(TAG, "WiFi disconnected from AP");
            state = Initialized;
            on_ap_disconnected();
        }
        return;
    }

    // STA mode started
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        LOG_DEBUG(TAG, "WiFi STA mode started, connecting ...");
        nb_retry_connect = 0;
        esp_wifi_connect();
        return;
    }

    // Connected to AP
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        // We don't trigger connected here, we wait for the IP address to be assigned
        return;
    }

    // AP Started
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START)
    {
        on_ap_started();
        return;
    }

    // AP Stopped
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP)
    {
        on_ap_stopped();
        return;
    }
    
    // Got IP Address from AP
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // Store IP in ip_addr attribute
        ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(event_data);
        sprintf(ip_addr, IPSTR, IP2STR(&event->ip_info.ip));
        LOG_DEBUG(TAG, "[EVENT] Got IP address from AP: %s", ip_addr);
        // Trigger connected event here
        on_ap_connected();
        return;
    }

    // AP Scan Done
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        if (!scan_callback)
        {
            LOG_WARNING(TAG, "WiFi scan done event received but no callback is set");
            return;
        }

        wifi_event_sta_scan_done_t* scan_done_event = static_cast<wifi_event_sta_scan_done_t*>(event_data);
        if (scan_done_event->status != 0)
        {
            LOG_ERROR(TAG, "WiFi scan failed with status %d", scan_done_event->status);
            scan_callback(nullptr, 0, scan_callback_user_data);
            scan_callback = nullptr;
            scan_callback_user_data = nullptr;
            return;
        }

        uint16_t ap_count = scan_done_event->number;
        wifi_ap_record_t* ap_records = new wifi_ap_record_t[ap_count];

        if (esp_wifi_scan_get_ap_records(&ap_count, ap_records) != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to get scanned AP records");
            delete[] ap_records;
            scan_callback(nullptr, 0, scan_callback_user_data);
            scan_callback = nullptr;
            scan_callback_user_data = nullptr;
            return;
        }

        APDescription* ap_list = new APDescription[ap_count];
        for (uint16_t i = 0; i < ap_count; ++i)
        {
            strncpy(ap_list[i].ssid, reinterpret_cast<const char*>(ap_records[i].ssid), sizeof(ap_list[i].ssid));
            ap_list[i].rssi = ap_records[i].rssi;
            ap_list[i].authmode = static_cast<APAuthMode>(ap_records[i].authmode); // direct cast is safe here because the enum values match
        }
        delete[] ap_records;

        scan_callback(ap_list, ap_count, scan_callback_user_data);
        scan_callback = nullptr;
        scan_callback_user_data = nullptr;
        delete[] ap_list;
    }

    // LOG_DEBUG(TAG, "Unhandled WiFi event: base=%s, id=%d", event_base, event_id);
}

WiFiManager::WiFiManager()
{
}

Status WiFiManager::init()
{
    LOG_SCOPE(TAG, "WiFiManager::init");

    state = State::Initializing;

    if (Status err = NVS::Init(); err != Status::Ok)
    {
        state = State::Error;
        return err;
    }
    if (Status err = NVS::Open("wifi", &nvs_handle_ptr); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to open NVS namespace for WiFi: %d", static_cast<int>(err));
        state = State::Error;
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return err;
    }

    if (esp_netif_init() != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_netif_init failed");
        state = State::Error;
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }
    if (esp_event_loop_create_default() != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_event_loop_create_default failed");
        state = State::Error;
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }

    // Create default WiFi station and AP interfaces
    sta_netif = esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_init failed");
        state = State::Error;
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }

    // Register event handlers
    if (esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this) != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to register WiFi event handler");
        state = State::Error;
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }
    if (esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, this) != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to register IP event handler");
        state = State::Error;
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }

    state = State::Initialized;
    return Status::Ok;
}

Status WiFiManager::deinit()
{
    if (nvs_handle_ptr)
    {
        NVS::Close(nvs_handle_ptr);
        nvs_handle_ptr = nullptr;
    }

    return Status::Ok;
}

Status WiFiManager::connectToAP(const char* _ssid, const char* _password, bool store_credentials, bool fallback_to_ap)
{
    if (__in_transition_state() || state == Error)
    {
        return Status::InvalidState;
    }

    if (state == Connected)
    {
        // If already connected, disconnect first
        if (Status err = __disconnect_from_ap(); err != Status::Ok)
        {
            return err;
        }
    }
    if (state == Started)
    {
        // If AP is started, stop it first
        if (Status err = stopAP(); err != Status::Ok)
        {
            return err;
        }
    }

    if (!_ssid || strlen(_ssid) == 0)
    {
        // SSID is empty, try to use stored credentials (ssid & password) from NVS
        __load_stored_credentials();
        if (!stored_credentials.has_credentials)
        {
            // No credentials stored? check if fallback to AP is allowed, if not, fail.
            if (fallback_to_ap)
            {
                LOG_WARNING(TAG, "No stored Wi-Fi credentials found, falling back to AP mode");
                return startAP();
            }
            else return Status::InvalidParameters;
        }
        return __connect_to_ap(stored_credentials.ssid, stored_credentials.password, false, fallback_to_ap);
    }

    return __connect_to_ap(_ssid, _password, store_credentials, fallback_to_ap);
}

Status WiFiManager::disconnectFromAP()
{
    if (state != Connected)
    {
        LOG_ERROR(TAG, "disconnectFromAP called in invalid state: %d", static_cast<int>(state));
        return Status::InvalidState;
    }

    return __disconnect_from_ap();
}

Status WiFiManager::hasStoredAPCredentials(bool &has_credentials)
{
    if (Status err = __load_stored_credentials(); err != Status::Ok)
    {
        return err;
    }

    has_credentials = stored_credentials.has_credentials;
    return Status::Ok;
}

Status WiFiManager::forgetStoredAPCredentials()
{
    return __erase_stored_credentials();
}

Status WiFiManager::startAP(const char* _ssid, const char* _password)
{
    if (__in_transition_state() || state == Error)
    {
        return Status::InvalidState;
    }

    if (state == Connected)
    {
        // If already connected, disconnect first
        if (Status err = __disconnect_from_ap(); err != Status::Ok)
        {
            return err;
        }
    }
    if (state == Started)
    {
        // If AP is started, stop it first
        if (Status err = stopAP(); err != Status::Ok)
        {
            return err;
        }
    }

    if (!_ssid || strlen(_ssid) == 0 || !_password)
    {
        LOG_ERROR(TAG, "startAP called with invalid parameters: ssid='%s', password='%s'", _ssid ? _ssid : "null", _password ? _password : "null");
        return Status::InvalidParameters;
    }

    return __start_ap(_ssid, _password);
}

Status WiFiManager::stopAP()
{
    if (state != Started)
    {
        LOG_ERROR(TAG, "stopAP called in invalid state: %d", static_cast<int>(state));
        return Status::InvalidState;
    }

    __stop_ap();
    return Status::Ok;
}

/// OTHER METHODS ///

Status WiFiManager::scanAP(OnAPScanResultCallback callback, void* user_data)
{
    if (__in_transition_state() || state == Error)
    {
        return Status::InvalidState;
    }

    if (scan_callback != nullptr)
    {
        LOG_WARNING(TAG, "scanAP called while a scan is already in progress");
        return Status::InvalidState;
    }
    scan_callback = callback;
    scan_callback_user_data = user_data;

    // Set in correct state for scanning
    if (state == Connected)
    {
        // If already connected, disconnect first
        if (Status err = __disconnect_from_ap(); err != Status::Ok)
        {
            return err;
        }
    }
    if (state == Started)
    {
        // If AP is started, stop it first
        if (Status err = stopAP(); err != Status::Ok)
        {
            return err;
        }
    }

    // Set WiFi mode to Station
    if (esp_err_t err = esp_wifi_set_mode(WIFI_MODE_STA); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_mode failed with code 0x%x", err);
        return Status::Failure;
    }

    wifi_config_t wifi_config = {};
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;

    // Set WiFi mode to Station
    if (esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_config failed with code 0x%x", err);
        return Status::Failure;
    }

    // Disable power save for better performance
    if (esp_err_t err = esp_wifi_set_ps(WIFI_PS_NONE); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_ps failed with code 0x%x", err);
        return Status::Failure;
    }
    
    // Start WiFi in Station mode
    if (esp_err_t err = esp_wifi_start(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_start failed with code 0x%x", err);
        return Status::Failure;
    }

    // Start scan in non-blocking mode, will trigger WIFI_EVENT_SCAN_DONE when done
    if (esp_err_t err = esp_wifi_scan_start(nullptr, false); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_scan_start failed with code 0x%x", err);
        scan_callback = nullptr;
        scan_callback_user_data = nullptr;
        return Status::Failure;
    }

    return Status::Ok;
}

/// INTERNAL METHODS ///

bool WiFiManager::__in_transition_state()
{
    return (state == Initializing || state == Connecting || state == Disconnecting || state == Starting || state == Stopping);
}

Status WiFiManager::__connect_to_ap(const char* _ssid, const char* _password, bool store_credentials, bool fallback_to_ap)
{
    // Copy ssid and password in config
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, _ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (_password && strlen(_password) > 0)
    {
        strncpy((char*)wifi_config.sta.password, _password, sizeof(wifi_config.sta.password) - 1);
    }
    wifi_config.sta.failure_retry_cnt = WIFI_MAX_RETRIES;

    // Set WiFi mode to Station
    if (esp_err_t err = esp_wifi_set_mode(WIFI_MODE_STA); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_mode failed with code 0x%x", err);
        return Status::Failure;
    }

    // Set WiFi configuration
    if (esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_config failed with code 0x%x", err);
        return Status::Failure;
    }

    // Disable power save for better performance
    if (esp_err_t err = esp_wifi_set_ps(WIFI_PS_NONE); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_ps failed with code 0x%x", err);
        return Status::Failure;
    }

    // Store credentials in temp_credentials for later use (e.g., storing in NVS after successful connection)
    strncpy(temp_credentials.ssid, _ssid, sizeof(temp_credentials.ssid) - 1);
    if (_password && strlen(_password) > 0)
        strncpy(temp_credentials.password, _password, sizeof(temp_credentials.password) - 1);
    else temp_credentials.password[0] = '\0'; // No password
    temp_credentials.should_store_credentials = store_credentials;
    should_fallback_to_ap = fallback_to_ap;

    on_ap_connecting();
    
    // Start WiFi in Station mode (it will now try to connect to the AP)
    if (esp_err_t err = esp_wifi_start(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_start failed with code 0x%x", err);
        return Status::Failure;
    }
    return Status::Ok;
}

Status WiFiManager::__disconnect_from_ap()
{
    on_ap_disconnecting();

    if (esp_err_t err = esp_wifi_disconnect(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_disconnect failed with code 0x%x", err);
        state = Error;
        return Status::Failure;
    }
    
    if (esp_wifi_stop() != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_stop failed");
        state = Error;
        return Status::Failure;
    }
    return Status::Ok;
}

Status WiFiManager::__start_ap(const char* _ssid, const char* _password)
{
    // Configure Access Point
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, _ssid, sizeof(wifi_config.ap.ssid) - 1);
    wifi_config.ap.ssid_len = strlen(_ssid);
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    if (strlen(_password) > 0)
    {
        strncpy((char*)wifi_config.ap.password, _password, sizeof(wifi_config.ap.password) - 1);
        wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }

    // Set WiFi mode to Access Point
    if (esp_err_t err = esp_wifi_set_mode(WIFI_MODE_AP); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_mode failed with code 0x%x", err);
        return Status::Failure;
    }
    
    // Set WiFi configuration
    if (esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_config failed with code 0x%x", err);
        return Status::Failure;
    }
    
    // Disable power save for better performance
    if (esp_err_t err = esp_wifi_set_ps(WIFI_PS_NONE); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_ps failed with code 0x%x", err);
        return Status::Failure;
    }

    temp_credentials.should_store_credentials = false; // No need to store AP credentials
    strncpy(temp_credentials.ssid, _ssid, sizeof(temp_credentials.ssid) - 1);
    if (_password && strlen(_password) > 0)
        strncpy(temp_credentials.password, _password, sizeof(temp_credentials.password) - 1);
    else temp_credentials.password[0] = '\0'; // No password
    
    on_ap_starting();

    // Start WiFi in Access Point mode (it will now try to start the AP)
    if (esp_err_t err = esp_wifi_start(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_start failed with code 0x%x", err);
        return Status::Failure;
    }
    return Status::Ok;
}

Status WiFiManager::__stop_ap()
{
    on_ap_stopping();

    // Stop WiFi in Access Point mode
    if (esp_err_t err = esp_wifi_stop(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_stop failed with code 0x%x", err);
        state = Error;
        return Status::Failure;
    }
    return Status::Ok;
}

/// GETTERS ///

WiFiManager::Mode WiFiManager::getMode() const
{
    if (state == State::Connecting || state == State::Connected || state == State::Disconnecting || state == State::ConnectFailed)
    {
        return Mode::STA;
    }
    if (state == State::Starting || state == State::Started || state == State::Stopping)
    {
        return Mode::AP;
    }
    return Mode::None;
}

/// NVS CREDENTIALS MANAGEMENT ///

Status WiFiManager::__load_stored_credentials()
{
    if (!nvs_handle_ptr)
    {
        LOG_ERROR(TAG, "NVS handle is null. WiFiManager not initialized?");
        return Status::InvalidState;
    }

    if (Status err = nvs_handle_ptr->get("ssid", stored_credentials.ssid); err != Status::Ok)
    {
        if (err == Status::NotFound)
        {
            stored_credentials.has_credentials = false;
            return Status::Ok;
        }
        LOG_ERROR(TAG, "Failed to read SSID from NVS: %d", static_cast<int>(err));
        return err;
    }

    if (Status err = nvs_handle_ptr->get("password", stored_credentials.password); err != Status::Ok)
    {
        if (err == Status::NotFound)
        {
            strcpy(stored_credentials.password, ""); // No password stored
        }
        else
        {
            LOG_ERROR(TAG, "Failed to read password from NVS: %d", static_cast<int>(err));
            return err;
        }
    }

    stored_credentials.has_credentials = true;
    return Status::Ok;
}

Status WiFiManager::__save_credentials()
{
    if (!nvs_handle_ptr)
    {
        LOG_ERROR(TAG, "NVS handle is null. WiFiManager not initialized?");
        return Status::InvalidState;
    }
    
    if (Status err = nvs_handle_ptr->set("ssid", temp_credentials.ssid); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to write SSID to NVS: %d", static_cast<int>(err));
        return err;
    }
    stored_credentials.has_credentials = true;
    strcpy(stored_credentials.ssid, temp_credentials.ssid);

    if (Status err = nvs_handle_ptr->set("password", temp_credentials.password); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to write password to NVS: %d", static_cast<int>(err));
        return err;
    }
    strcpy(stored_credentials.password, temp_credentials.password);
    return Status::Ok;
}

Status WiFiManager::__erase_stored_credentials()
{
    if (!nvs_handle_ptr)
    {
        LOG_ERROR(TAG, "NVS handle is null. WiFiManager not initialized?");
        return Status::InvalidState;
    }

    if (Status err = nvs_handle_ptr->erase("ssid"); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to erase SSID from NVS: %d", static_cast<int>(err));
        return err;
    }
    if (Status err = nvs_handle_ptr->erase("password"); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to erase password from NVS: %d", static_cast<int>(err));
        return err;
    }
    stored_credentials.has_credentials = false;
    return Status::Ok;
}

/// WIFI EVENT CALLBACKS ///

void WiFiManager::on_ap_connecting()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi connecting to AP");
    state = Connecting; // Will transition to Connected or ConnectFailed state in wifi event callback after connection attempt
}

void WiFiManager::on_ap_connected()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi connected to AP");
    state = Connected;

    if (temp_credentials.should_store_credentials)
    {
        LOG_DEBUG(TAG, "Storing WiFi credentials to NVS");
        if (Status err = __save_credentials(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to save WiFi credentials to NVS: %d", static_cast<int>(err));
        }
    }
}

void WiFiManager::on_ap_disconnecting()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi disconnecting from AP");
    state = Disconnecting; // Will transition to Initialized state in wifi event callback after disconnection is complete
    strncpy(ip_addr, "", sizeof(ip_addr)); // Clear IP address
}

void WiFiManager::on_ap_disconnected()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi disconnected from AP");
    state = Initialized;
}

void WiFiManager::on_ap_failed_connect()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi failed to connect to AP");
    if (should_fallback_to_ap)
    {
        LOG_DEBUG(TAG, "Falling back to AP mode");
        if (Status err = __start_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to start AP after failed connection: %d", static_cast<int>(err));
        }
    }
    else state = ConnectFailed;
}

void WiFiManager::on_ap_starting()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi AP starting");
    state = Starting; // Will transition to Started state in wifi event callback after AP is started
}

void WiFiManager::on_ap_started()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi AP started");
    state = Started;
    sprintf(ip_addr, "192.168.4.1"); // Default AP IP
}

void WiFiManager::on_ap_stopping()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi AP stopping");
    state = Stopping; // Will transition to Initialized state in wifi event callback after AP is stopped
    ip_addr[0] = '\0'; // Clear IP address
}

void WiFiManager::on_ap_stopped()
{
    LOG_DEBUG(TAG, "[EVENT] WiFi AP stopped");
    state = Initialized;
}
