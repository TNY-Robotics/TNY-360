#pragma once
#include "common/utils.hpp"
#include "network/WiFiManager.hpp"
#include "network/WebInterface.hpp"
#include "network/WebSocket.hpp"

class NetworkManager
{
public:
    NetworkManager();

    /**
     * @brief Initialize the network manager.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the network manager.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Get the WiFiManager instance.
     * @return Reference to the WiFiManager instance.
     */
    WiFiManager& getWiFiManager() { return wifi_manager; }

    /**
     * @brief Get the WebInterface instance.
     * @return Reference to the WebInterface instance.
     */
    WebInterface& getWebInterface() { return web_interface; }

    /**
     * @brief Get the WebSocket instance.
     * @return Reference to the WebSocket instance.
     */
    WebSocket& getWebSocket() { return web_socket; }

private:
    WiFiManager wifi_manager;
    WebInterface web_interface;
    WebSocket web_socket;
};