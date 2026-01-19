#pragma once
#include "common/utils.hpp"
#include "esp_http_server.h"
#include "common/config.hpp"

class WebSocket
{
public:
    constexpr static const char* TAG = "WebSocket";

    WebSocket(uint16_t port = 5621);

    /**
     * @brief Initialize the WebSocket.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the WebSocket.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Find a pending request by its ID.
     * @param id The ID of the pending request.
     * @param req Pointer to store the found request.
     * @return Error code indicating success or failure.
     */
    Error find_pending_request(uint16_t id, httpd_req_t** req);

    /**
     * @brief Internal WebSocket handler.
     * @note Do not call this function directly.
     */
    esp_err_t __ws_handler(httpd_req_t* req);

private:
    typedef struct PendingRequests
    {
        uint16_t id;
        httpd_req_t* req;
    } PendingRequests;

    PendingRequests pending_requests[PROTOCOL_MAX_PENDING_COMMANDS];
    uint16_t server_port;
    int nb_connected_clients = 0;
    httpd_handle_t server_handle = nullptr;

    void on_connected();
    void on_disconnected();
    Error register_uri_handlers();
};
