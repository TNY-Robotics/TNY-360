#include "network/WebSocket.hpp"
#include "common/Log.hpp"
#include "network/Protocol.hpp"
#include "Robot.hpp"
#include <esp_wifi.h>

WebSocket::WebSocket(uint16_t port) : server_port(port)
{
}

Error WebSocket::init()
{
    // create the web server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = server_port;
    config.max_uri_handlers = 8;
    config.ctrl_port = server_port + 1;

    // start the server
    if (httpd_start(&server_handle, &config) != ESP_OK)
    {
        server_handle = nullptr;
        Log::Add(Log::Level::Error, TAG, "Failed to start WebSocket server");
        return Error::Unknown;
    }

    // register URI handlers and return
    return register_uri_handlers();
}

Error WebSocket::deinit()
{
    if (server_handle)
    {
        httpd_stop(server_handle);
        server_handle = nullptr;
    }
    return Error::None;
}

Error WebSocket::register_uri_handlers()
{
    httpd_uri_t ws = {
        .uri        = "/",
        .method     = HTTP_GET,
        .handler    = [](httpd_req_t* req) {
            return Robot::GetInstance().getNetworkManager().getWebSocket().__ws_handler(req);
        },
        .user_ctx   = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = ""
    };

    httpd_register_uri_handler((httpd_handle_t) server_handle, &ws);
    esp_event_handler_register(ESP_HTTP_SERVER_EVENT, HTTP_SERVER_EVENT_ON_CONNECTED,
        [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
            Robot::GetInstance().getNetworkManager().getWebSocket().on_connected();
        }, nullptr
    );
    esp_event_handler_register(ESP_HTTP_SERVER_EVENT, HTTP_SERVER_EVENT_DISCONNECTED,
        [](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
            Robot::GetInstance().getNetworkManager().getWebSocket().on_disconnected();
        }, nullptr
    );

    return Error::None;
}

void WebSocket::on_connected()
{
    nb_connected_clients++;
    if (nb_connected_clients == 1) {
        // First client connected, disable low power mode
        Log::Add(Log::Level::Info, TAG, "WebSocket client connected, disabling Wi-Fi low power mode.");
        esp_wifi_set_ps(WIFI_PS_NONE);
    }
}

void WebSocket::on_disconnected()
{
    nb_connected_clients--;
    if (nb_connected_clients < 0) {
        nb_connected_clients = 0; // Ensure the count doesn't go negative
    }

    if (nb_connected_clients == 0) {
        // No client connected, activate low power mode
        Log::Add(Log::Level::Info, TAG, "No WebSocket clients connected, switching Wi-Fi to low power mode.");
        esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    }
}

esp_err_t WebSocket::__ws_handler(httpd_req_t* req)
{
    if (req->method == HTTP_GET) {
        // Handshake request, nothing to do
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        Log::Add(Log::Level::Error, TAG, "WebSocket receive error: %d", ret);
        return ret;
    }

    if (ws_pkt.len == 0) {
        Log::Add(Log::Level::Debug, TAG, "Empty WebSocket frame received, discarding");
        return ESP_OK;
    }

    if (ws_pkt.len > WEBSOCKET_MAX_MSG_SIZE) {
        Log::Add(Log::Level::Warning, TAG, "WebSocket message too large: %d bytes", ws_pkt.len);
        return ESP_ERR_NO_MEM;
    }

    ws_pkt.payload = (uint8_t*) malloc(ws_pkt.len + 1);
    if (!ws_pkt.payload) {
        Log::Add(Log::Level::Error, TAG, "Memory allocation failed");
        return ESP_ERR_NO_MEM;
    }

    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret == ESP_OK) {
        ws_pkt.payload[ws_pkt.len] = 0;  // Null-terminate the payload for safety

        Protocol::Request req_pkt;
        req_pkt.id = (ws_pkt.payload[0] << 8) | ws_pkt.payload[1];
        req_pkt.cmd = ws_pkt.payload[2];
        uint8_t header_size = sizeof(req_pkt.id) + sizeof(req_pkt.cmd);
        req_pkt.len = ws_pkt.len - header_size;
        req_pkt.payload = &ws_pkt.payload[header_size];

        // Store the pending request
        bool stored = false;
        for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_COMMANDS; ++i) {
            if (pending_requests[i].id == 0) {
                pending_requests[i].id = req_pkt.id;
                pending_requests[i].req = req;
                stored = true;
                break;
            }
        }
        if (!stored) {
            Log::Add(Log::Level::Error, TAG, "No space to store pending request ID: %d", req_pkt.id);
            free(ws_pkt.payload);
            return ESP_ERR_NO_MEM;
        }

        Error err = Robot::GetInstance().getProtocol().handleRequest(req_pkt, [](const Protocol::Response& res) {
            httpd_req_t* req;
            if (Robot::GetInstance().getNetworkManager().getWebSocket().find_pending_request(res.id, &req) != Error::None) {
                Log::Add(Log::Level::Warning, TAG, "No pending request found for response ID: %d", res.id);
                return;
            }

            httpd_ws_frame_t ws_res;
            memset(&ws_res, 0, sizeof(httpd_ws_frame_t));
            ws_res.payload = (uint8_t*) malloc(sizeof(res.id) + sizeof(res.status) + sizeof(res.len) + res.len);
            if (!ws_res.payload) {
                Log::Add(Log::Level::Error, TAG, "Memory allocation failed for response");
                return;
            }
            ws_res.len = sizeof(res.id) + sizeof(res.status) + sizeof(res.len) + res.len;
            ws_res.type = HTTPD_WS_TYPE_BINARY;
            ws_res.payload[0] = (res.id >> 8) & 0xFF;
            ws_res.payload[1] = res.id & 0xFF;
            ws_res.payload[2] = res.status;
            ws_res.payload[3] = res.len;
            memcpy(&ws_res.payload[4], res.payload, res.len);
            esp_err_t ret = httpd_ws_send_frame(req, &ws_res);
            if (ret != ESP_OK) {
                Log::Add(Log::Level::Error, TAG, "WebSocket send error: %d", ret);
            }
            free(ws_res.payload);
        });

        if (err != Error::None) {
            Log::Add(Log::Level::Error, TAG, "Protocol::Handle error: %d", static_cast<uint8_t>(err));
            char hex_str[WEBSOCKET_MAX_MSG_SIZE * 3 + 1] = {0}; // Each byte: "XX " + null terminator
            size_t pos = 0;
            for (size_t i = 0; i < ws_pkt.len && pos < sizeof(hex_str) - 4; ++i) {
                pos += snprintf(hex_str + pos, sizeof(hex_str) - pos, "%02X ", ws_pkt.payload[i]);
            }
            Log::Add(Log::Level::Debug, TAG, "Request command: 0x%02X, length: %d, payload (hex): %s", req_pkt.cmd, req_pkt.len, hex_str);
            ret = ESP_FAIL;
        }
    }

    free(ws_pkt.payload);
    return ret;
}

Error WebSocket::find_pending_request(uint16_t id, httpd_req_t** req)
{
    for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_COMMANDS; ++i) {
        if (pending_requests[i].id == id) {
            *req = pending_requests[i].req;
            // Clear the pending request entry (mark as free)
            pending_requests[i].id = 0;
            pending_requests[i].req = nullptr;
            return Error::None;
        }
    }
    return Error::NotFound;
}