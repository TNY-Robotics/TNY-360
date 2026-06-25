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
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        LOG_DEBUG(TAG, "STA started, attempting to connect");
        esp_wifi_connect();
        mode = Station;
        state = Connecting;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        this->state = WiFiManager::Disconnected;
        memset(ip_address, 0, sizeof(ip_address));
        LOG_DEBUG(TAG, "WiFi disconnected, reason: %d", ((wifi_event_sta_disconnected_t*)event_data)->reason);

        if (retry_count < WIFI_MAX_RETRIES)
        {
            esp_wifi_connect();
            retry_count++;
            LOG_DEBUG(TAG, "Reconnecting to WiFi (attempt %d)", retry_count);
        }
        else
        {
            LOG_DEBUG(TAG, "Failed to connect to WiFi after %d attempts", WIFI_MAX_RETRIES);
            mode = Station;
            state = Disconnected;
            on_ap_disconnected();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        mode = Station;
        state = Connected;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        snprintf(ip_address, sizeof(ip_address), IPSTR, IP2STR(&event->ip_info.ip));
        LOG_DEBUG(TAG, "WiFi connected, IP obtained (%s)", ip_address);

        // store if needed
        if (should_store_credentials)
        {
            if (!nvs_handle_ptr)
            {
                LOG_ERROR(TAG, "NVS handle not available, cannot store credentials");
                return;
            }

            if (strlen(ssid) > 0)
            {
                if (Status err = nvs_handle_ptr->set("ssid", ssid, strlen(ssid) + 1); err != Status::Ok)
                {
                    LOG_ERROR(TAG, "Failed to store SSID in NVS");
                }
                else
                {
                    LOG_DEBUG(TAG, "SSID stored in NVS");
                }
            }

            if (strlen(password) > 0)
            {
                if (Status err = nvs_handle_ptr->set("password", password, strlen(password) + 1); err != Status::Ok)
                {
                    LOG_ERROR(TAG, "Failed to store password in NVS");
                }
                else
                {
                    LOG_DEBUG(TAG, "Password stored in NVS");
                }
            }
        }
        
        retry_count = 0;
        on_ap_connected();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START)
    {
        mode = AccessPoint;
        state = Connected;
        LOG_DEBUG(TAG, "WiFi AP started");
        sprintf(ip_address, "192.168.4.1"); // Default AP IP
        on_ap_started();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP)
    {
        mode = AccessPoint;
        state = Disconnected;
        LOG_DEBUG(TAG, "WiFi AP stopped");
        memset(ip_address, 0, sizeof(ip_address));
        on_ap_stopped();
    }
}

WiFiManager::WiFiManager()
{
    state = Disconnected;
    mode = Station;
}

Status WiFiManager::init()
{
    LOG_SCOPE(TAG, "WiFiManager::init");

    if (Status err = NVS::Init(); err != Status::Ok)
    {
        return err;
    }

    if (esp_netif_init() != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_netif_init failed");
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }
    if (esp_event_loop_create_default() != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_event_loop_create_default failed");
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
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }

    // Register event handlers
    if (esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this) != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to register WiFi event handler");
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }
    if (esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, this) != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to register IP event handler");
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return Status::Failure;
    }

    if (Status err = NVS::Open("WiFi", &nvs_handle_ptr); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to open NVS namespace for WiFi: %d", static_cast<int>(err));
        // ErrorHandle(ErrorStruct::WiFiInitFailed);
        return err;
    }
    
    if (nvs_handle_ptr->get("ssid", ssid, sizeof(ssid)) != Status::Ok)
    {
        LOG_DEBUG(TAG, "No stored SSID found in NVS");
        ssid[0] = '\0';
    }
    if (nvs_handle_ptr->get("password", password, sizeof(password)) != Status::Ok)
    {
        LOG_DEBUG(TAG, "No stored password found in NVS");
        password[0] = '\0';
    }

    if (strlen(ssid) > 0)
    {
        LOG_DEBUG(TAG, "Connecting to stored SSID: %s (%s)", ssid, password);
        __connect_to_ap(); // Attempt to connect to stored SSID
    }
    else
    {
        // No stored SSID, start AP mode
        LOG_DEBUG(TAG, "No SSID stored. Starting AP");
        strcpy(ssid, WIFI_AP_SSID);
        strcpy(password, WIFI_AP_PASSWORD);
        if (Status err = __create_ap(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Create AP Failed");
            // ErrorHandle(ErrorStruct::WiFiInitFailed);
            return err;
        }
    }

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

Status WiFiManager::connect(const char* _ssid, const char* _password, bool store_credentials)
{
    if (strlen(_ssid) == 0)
    {
        return Status::InvalidParameters;
    }

    strncpy(ssid, _ssid, sizeof(ssid) - 1);
    strncpy(password, _password, sizeof(password) - 1);
    should_store_credentials = store_credentials;
    __connect_to_ap();

    return Status::Ok;
}

Status WiFiManager::disconnect()
{
    if (state != Connected)
    {
        return Status::InvalidState;
    }

    return Status::Ok;
}

Status WiFiManager::createAccessPoint(const char* _ssid, const char* _password)
{
    if (strlen(_ssid) == 0)
    {
        return Status::InvalidParameters;
    }

    strncpy(ssid, _ssid, sizeof(ssid) - 1);
    strncpy(password, _password, sizeof(password) - 1);

    __create_ap();
    return Status::Ok;
}


Status WiFiManager::__connect_to_ap()
{
    // Stop current WiFi mode if running
    if (esp_wifi_stop() != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_stop failed");
        return Status::Failure;
    }

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (strlen(password) > 0) {
        strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
        // wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }
    
    // WPA3/WPA2-mixed
    // wifi_config.sta.pmf_cfg.capable = true;
    // wifi_config.sta.pmf_cfg.required = false;

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
    
    if (esp_err_t err = esp_wifi_start(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_start failed with code 0x%x", err);
        return Status::Failure;
    }
    
    esp_wifi_set_max_tx_power(40);
    
    state = Connecting;

    return Status::Ok;
}

Status WiFiManager::__create_ap()
{
    // Stop current WiFi mode if running
    if (esp_wifi_stop() != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_stop failed");
        return Status::Failure;
    }

    // Configure Access Point
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    wifi_config.ap.ssid_len = strlen(ssid);
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    if (strlen(password) > 0) {
        strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password) - 1);
        wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }

    if (esp_err_t err = esp_wifi_set_mode(WIFI_MODE_AP); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_mode failed with code 0x%x", err);
        return Status::Failure;
    }
    if (esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_set_config failed with code 0x%x", err);
        return Status::Failure;
    }
    if (esp_err_t err = esp_wifi_start(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "esp_wifi_start failed with code 0x%x", err);
        return Status::Failure;
    }

    esp_wifi_set_max_tx_power(40);
    
    LOG_DEBUG(TAG, "Starting AP with config: ssid=%s, password=%s", wifi_config.ap.ssid, wifi_config.ap.password);

    return Status::Ok;
}

void WiFiManager::on_ap_started()
{
    LOG_DEBUG(TAG, "WiFiManager::on_ap_started");
    if (Status err = dns_server.init(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to start DNS server");
    }
    LOG_INFO(TAG, "Access Point started. SSID: %s, IP: %s", ssid, ip_address);
}

void WiFiManager::on_ap_stopped()
{
    if (Status err = dns_server.deinit(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to stop DNS server");
    }
}

void WiFiManager::on_ap_connected()
{
}

void WiFiManager::on_ap_disconnected()
{
    LOG_DEBUG(TAG, "Disconnected from Access Point. Switching to AP mode");
    strcpy(ssid, WIFI_AP_SSID);
    strcpy(password, WIFI_AP_PASSWORD);
    __create_ap();
}
