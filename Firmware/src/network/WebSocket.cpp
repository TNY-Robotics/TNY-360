#include "network/WebSocket.hpp"
#include "common/Log.hpp"
#include "network/Protocol.hpp"
#include "Robot.hpp"
#include <esp_wifi.h>

namespace WebSocketUtils
{
    struct AsyncWebsocketResponse {
        httpd_handle_t hd;
        int fd;
        uint8_t* payload;
        size_t len;
    };

    static void ws_async_send_worker(void *arg) {
        AsyncWebsocketResponse* resp = static_cast<AsyncWebsocketResponse*>(arg);
        
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.type = HTTPD_WS_TYPE_BINARY;
        ws_pkt.payload = resp->payload;
        ws_pkt.len = resp->len;
        ws_pkt.final = true;
        
        esp_err_t err = httpd_ws_send_frame_async(resp->hd, resp->fd, &ws_pkt);
        if (err != ESP_OK) {
            Log::Add(Log::Level::Error, "WebSocket", "Async send error: %d", err);
        }
        
        free(resp->payload);
        delete resp;
    }
}

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
    config.max_open_sockets = 3;
    // config.lru_purge_enable = true;

    // create the mutex
    pending_requests_mutex = xSemaphoreCreateMutex();

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

esp_err_t WebSocket::__ws_handler(httpd_req_t* ws_req)
{
    if (ws_req->method == HTTP_GET) {
        // Handshake request, nothing to do
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    
    esp_err_t ret = httpd_ws_recv_frame(ws_req, &ws_pkt, 0);
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

    ret = httpd_ws_recv_frame(ws_req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
        Log::Add(Log::Level::Error, TAG, "Error receiving websocket frame : 0x%X", ret);
        free(ws_pkt.payload);
        return ESP_FAIL;
    }

    ws_pkt.payload[ws_pkt.len] = 0;  // Null-terminate the payload for safety

    Protocol::Request req;
    req.id = (ws_pkt.payload[0] << 8) | ws_pkt.payload[1];
    req.cmd = ws_pkt.payload[2];
    uint8_t header_size = sizeof(req.id) + sizeof(req.cmd);
    req.len = ws_pkt.len - header_size;
    req.payload = &ws_pkt.payload[header_size];

    // Extract the handle and file descriptor
    httpd_handle_t hd = ws_req->handle;
    int fd = httpd_req_to_sockfd(ws_req);

    // Store the pending request
    bool stored = false;
    xSemaphoreTake(pending_requests_mutex, portMAX_DELAY);
    for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_COMMANDS; ++i) {
        if (pending_requests[i].id == 0) {
            pending_requests[i].id = req.id;
            pending_requests[i].hd = hd; // Store handle
            pending_requests[i].fd = fd; // Store file descriptor
            stored = true;
            break;
        }
    }
    xSemaphoreGive(pending_requests_mutex);

    if (!stored) {
        Log::Add(Log::Level::Error, TAG, "No space to store pending request ID: %d", req.id);

        // Respond error directly
        size_t payload_len = sizeof(Protocol::Response::id) + sizeof(Protocol::Response::status);
        uint8_t* payload = (uint8_t*) malloc(payload_len);
        payload[0] = (req.id >> 8) & 0xFF;
        payload[1] = req.id & 0xFF;
        payload[2] = 0; // TODO : Replace with proper CHILL OUT error code
        WebSocketUtils::AsyncWebsocketResponse* work_arg = new WebSocketUtils::AsyncWebsocketResponse{
            .hd = hd,
            .fd = fd,
            .payload = payload,
            .len = payload_len
        };
        if (httpd_queue_work(hd, WebSocketUtils::ws_async_send_worker, work_arg) != ESP_OK) {
            Log::Add(Log::Level::Error, TAG, "Failed to queue work to HTTPD thread");
            free(payload);
            delete work_arg;
        }
        
        free(ws_pkt.payload);
        return ESP_OK;
    }

    Error err = Robot::GetInstance().getNetworkManager().getProtocol().handleRequest(req, [](const Protocol::Response& res) {
        httpd_handle_t hd; int fd;
        if (Robot::GetInstance().getNetworkManager().getWebSocket().find_pending_request(res.id, &hd, &fd) != Error::None) {
            Log::Add(Log::Level::Warning, TAG, "No pending request found for response ID: %d", res.id);
            if (res.payload) free(res.payload);
            return;
        }

        size_t total_len = sizeof(res.id) + sizeof(res.status) + sizeof(res.len) + res.len;
        uint8_t* payload_buffer = (uint8_t*) malloc(total_len);

        if (!payload_buffer) {
            Log::Add(Log::Level::Error, WebSocket::TAG, "Memory allocation failed for response");
            return;
        }

        payload_buffer[0] = (res.id >> 8) & 0xFF;
        payload_buffer[1] = res.id & 0xFF;
        payload_buffer[2] = res.status;
        payload_buffer[3] = res.len;
        if (res.len > 0 && res.payload != nullptr) {
            memcpy(&payload_buffer[4], res.payload, res.len);
        }
        WebSocketUtils::AsyncWebsocketResponse* work_arg = new WebSocketUtils::AsyncWebsocketResponse{
            .hd = hd,
            .fd = fd,
            .payload = payload_buffer,
            .len = total_len
        };
        if (httpd_queue_work(hd, WebSocketUtils::ws_async_send_worker, work_arg) != ESP_OK) {
            Log::Add(Log::Level::Error, TAG, "Failed to queue work to HTTPD thread");
            free(payload_buffer);
            delete work_arg;
        }
    });

    if (err != Error::None) {
        Log::Add(Log::Level::Error, TAG, "Protocol::Handle error: %s", ErrorToString(err));
        char hex_str[WEBSOCKET_MAX_MSG_SIZE * 3 + 1] = {0}; // Each byte: "XX " + null terminator
        size_t pos = 0;
        for (size_t i = 0; i < ws_pkt.len && pos < sizeof(hex_str) - 4; ++i) {
            pos += snprintf(hex_str + pos, sizeof(hex_str) - pos, "%02X ", ws_pkt.payload[i]);
        }
        Log::Add(Log::Level::Debug, TAG, "Request command: 0x%02X, length: %d, payload (hex): %s", req.cmd, req.len, hex_str);
        
        // Respond error directly
        size_t payload_len = sizeof(Protocol::Response::id) + sizeof(Protocol::Response::status);
        uint8_t* payload = (uint8_t*) malloc(payload_len);
        payload[0] = (req.id >> 8) & 0xFF;
        payload[1] = req.id & 0xFF;
        payload[2] = 0; // TODO : Replace with proper UNKNOWN error code
        WebSocketUtils::AsyncWebsocketResponse* work_arg = new WebSocketUtils::AsyncWebsocketResponse{
            .hd = hd,
            .fd = fd,
            .payload = payload,
            .len = payload_len
        };
        if (httpd_queue_work(hd, WebSocketUtils::ws_async_send_worker, work_arg) != ESP_OK) {
            Log::Add(Log::Level::Error, TAG, "Failed to queue work to HTTPD thread");
            free(payload);
            delete work_arg;
        }
    }

    free(ws_pkt.payload);
    return ESP_OK;
}

Error WebSocket::find_pending_request(uint16_t id, httpd_handle_t* hd, int* fd)
{
    xSemaphoreTake(pending_requests_mutex, portMAX_DELAY);
    for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_COMMANDS; ++i) {
        if (pending_requests[i].id == id) {
            *hd = pending_requests[i].hd;
            *fd = pending_requests[i].fd;
            // Clear the pending request entry (mark as free)
            pending_requests[i].id = 0;
            xSemaphoreGive(pending_requests_mutex);
            return Error::None;
        }
    }
    xSemaphoreGive(pending_requests_mutex);
    return Error::NotFound;
}