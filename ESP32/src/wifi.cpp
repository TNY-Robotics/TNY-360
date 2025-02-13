#include <string.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include "storage.hpp"
#include "wifi.hpp"
#include "websocket.hpp"

#define TAG "wifi"
#define MAX_NB_RETRIES 5

uint8_t nb_retries = 0;
wifi_status_t wifi_status = WIFI_DISCONNECTED;
char wifi_ssid[WIFI_SSID_LENGTH];
uint32_t wifi_ip;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (nb_retries < MAX_NB_RETRIES)
        {
            nb_retries++;
            esp_wifi_connect();
        } else
        {
            wifi_status = WIFI_ERROR;
            ESP_LOGI(TAG, "Failed to connect to the AP");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        nb_retries = 0;
        wifi_status = WIFI_CONNECTED;
        wifi_ip = event->ip_info.ip.addr;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        // all good, start the websocket server
        websocket_start();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        wifi_event_sta_connected_t* wifi_event_data = (wifi_event_sta_connected_t*) event_data;
        wifi_status = WIFI_CONNECTED;
        strncpy(wifi_ssid, (char*)wifi_event_data->ssid, WIFI_SSID_LENGTH);
        ESP_LOGI(TAG, "Connected to %s", wifi_ssid);
    } else if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_STOP || event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        ESP_LOGI(TAG, "Disconnected from %s", wifi_ssid);
        wifi_ip = 0;
        wifi_ssid[0] = '\0';
        wifi_status = WIFI_DISCONNECTED;
        // stop the websocket server
        websocket_stop();
    }
}

esp_err_t wifi_init()
{
    wifi_ip = 0;
    memset(wifi_ssid, 0, WIFI_SSID_LENGTH);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t* netif_handle = esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_netif_set_hostname(netif_handle, "TNY - 360"));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    return ESP_OK;
}

esp_err_t wifi_connect()
{
    char ssid[WIFI_SSID_LENGTH];
    char pass[WIFI_PASSPHRASE_LENGTH];
    size_t ssid_len = WIFI_SSID_LENGTH;
    size_t pass_len = WIFI_PASSPHRASE_LENGTH;
    esp_err_t err;
    err = storage_get_str("wifi_ssid", ssid, &ssid_len);
    if (err != ESP_OK) return err;
    err = storage_get_str("wifi_pass", pass, &pass_len);
    if (err != ESP_OK) return err;
    return wifi_connect(ssid, pass);
}

esp_err_t wifi_connect(char* ssid, char* password)
{
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    wifi_status = WIFI_CONNECTING;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ESP_OK;
}

esp_err_t wifi_disconnect()
{
    if (wifi_status != WIFI_CONNECTED)
    {
        return ESP_OK;
    }

    wifi_status = WIFI_DISCONNECTING;
    ESP_ERROR_CHECK(esp_wifi_stop());
    return ESP_OK;
}

wifi_status_t wifi_get_status()
{
    return wifi_status;
}

char* wifi_get_ssid()
{
    return wifi_ssid;
}

uint32_t wifi_get_ip()
{
    return wifi_ip;
}
