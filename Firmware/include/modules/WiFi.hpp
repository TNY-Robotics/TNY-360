#pragma once
#include "utils.hpp"

namespace WiFi
{
    using WiFiCallbackThen = void(*)();
    using WiFiCallbackCatch = void(*)(Error);

    /**
     * @brief Initialize the WiFi module
     */
    Error Init();

    /**
     * @brief Connect to a previously configured WiFi network
     * @param cb_then Callback function to call on successful connection
     * @param cb_catch Callback function to call on connection failure
     */
    void Connect(WiFiCallbackThen cb_then = nullptr, WiFiCallbackCatch cb_catch = nullptr);

    /**
     * @brief Connect to a WiFi network
     * @param ssid The SSID of the network to connect to
     * @param password The password of the network
     * @param cb_then Callback function to call on successful connection
     * @param cb_catch Callback function to call on connection failure
     * @param save Whether to save the SSID and password to NVS (default: true)
     * @note SSID and password will only be saved if connection is successful
     */
    void Connect(const char* ssid, const char* password, WiFiCallbackThen cb_then = nullptr, WiFiCallbackCatch cb_catch = nullptr, bool save = true);

    /**
     * @brief Disconnect from the current WiFi network
     */
    void Disconnect();

    /**
     * @brief Check if currently connected to a WiFi network
     * @return true if connected, false otherwise
     */
    bool IsConnected();

    /**
     * @brief Get the current IP address
     * @return The IP address as a string, or nullptr if not connected
     */
    const char* GetIP();

    /**
     * @brief Get the MAC address of the WiFi interface
     * @return The MAC address as a string
     */
    const char* GetMAC();

    /**
     * @brief Start WiFi Access Point mode
     * @param ssid The SSID for the Access Point
     * @param password The password for the Access Point
     * @param cb_then Callback function to call on successful start
     * @param cb_catch Callback function to call on failure
     */
    void StartAP(const char* ssid, const char* password, WiFiCallbackThen cb_then = nullptr, WiFiCallbackCatch cb_catch = nullptr);

    /**
     * @brief Stop the WiFi Access Point
     */
    void StopAP();

    /**
     * @brief Check if Access Point is currently started
     * @return true if AP is started, false otherwise
     */
    bool IsAPStarted();
}
