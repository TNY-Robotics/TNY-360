#pragma once
#include "esp_err.h"

#define WIFI_SSID_LENGTH 32
#define WIFI_PASSPHRASE_LENGTH 64

/** Wifi status */
typedef enum {
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_DISCONNECTING,
    WIFI_ERROR
} wifi_status_t;

/** Initializes wifi */
esp_err_t wifi_init();

/** Connects to a wifi AP using stored ssid and password */
esp_err_t wifi_connect();

/** Connects to a wifi AP using given ssid and password */
esp_err_t wifi_connect(char* ssid, char* password);

/** Disconnects from the wifi AP */
esp_err_t wifi_disconnect();

/** Gets the current wifi status */
wifi_status_t wifi_get_status();

/** Gets the current wifi ssid */
char* wifi_get_ssid();

/** Gets the current wifi ip address */
uint32_t wifi_get_ip();
