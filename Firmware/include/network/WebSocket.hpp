#pragma once
#include "esp_http_server.h"
#include <freertos/semphr.h>
#include "common/utils.hpp"
#include "common/config.hpp"
#include "network/protocol/Protocol.hpp"

class WebSocket : public Protocol::ITransport
{
public:
    constexpr static const char* TAG = "WebSocket";

    WebSocket(uint16_t port = 5621);

    /**
     * @brief Initialize the WebSocket.
     * @return Error code indicating success or failure.
     */
    Status init();

    /**
     * @brief Deinitialize the WebSocket.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    void sendResponse(void* context, const Protocol::MessageHeader& header, const uint8_t* payload);

    uint8_t getNbClients() const;

private:
    esp_err_t ws_handler(httpd_req_t* req);

    uint16_t server_port;
    httpd_handle_t server_handle = nullptr;

    Status register_uri_handlers();
};
