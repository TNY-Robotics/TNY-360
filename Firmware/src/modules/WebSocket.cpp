#include "modules/WebSocket.hpp"
#include "modules/Log.hpp"
#include "modules/Protocol.hpp"
#include "modules/WebServer.hpp"
#include "esp_http_server.h"

namespace WebSocket
{
    typedef struct PendingRequests
    {
        uint16_t id;
        httpd_req_t* req;
    } PendingRequests;

    PendingRequests pending_requests[PROTOCOL_MAX_PENDING_CALLS] = {0};

    Error FindPendingRequest(uint16_t id, httpd_req_t** out_req)
    {
        for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_CALLS; ++i) {
            if (pending_requests[i].id == id) {
                *out_req = pending_requests[i].req;
                pending_requests[i].id = 0; // Mark as free
                pending_requests[i].req = nullptr;
                return Error::Ok;
            }
        }
        return Error::NotFound;
    }

    esp_err_t ws_handler(httpd_req_t *req)
    {
        if (req->method == HTTP_GET) {
            // Handshake request, nothing to do
            return ESP_OK;
        }

        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        
        esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
        if (ret != ESP_OK) {
            Log::Add(Log::Level::Error, "WebSocket receive error: %d", ret);
            return ret;
        }

        if (ws_pkt.len == 0) {
            Log::Add(Log::Level::Debug, "Empty WebSocket frame received, discarding");
            return ESP_OK;
        }

        if (ws_pkt.len > WEBSOCKET_MAX_MSG_SIZE) {
            Log::Add(Log::Level::Warning, "WebSocket message too large: %d bytes", ws_pkt.len);
            return ESP_ERR_NO_MEM;
        }

        ws_pkt.payload = (uint8_t*) malloc(ws_pkt.len + 1);
        if (!ws_pkt.payload) {
            Log::Add(Log::Level::Error, "Memory allocation failed");
            return ESP_ERR_NO_MEM;
        }

        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret == ESP_OK) {
            ws_pkt.payload[ws_pkt.len] = 0;  // Null-terminate pour éviter les erreurs

            char hex_str[WEBSOCKET_MAX_MSG_SIZE * 3 + 1] = {0}; // Each byte: "XX " + null terminator
            size_t pos = 0;
            for (size_t i = 0; i < ws_pkt.len && pos < sizeof(hex_str) - 4; ++i) {
                pos += snprintf(hex_str + pos, sizeof(hex_str) - pos, "%02X ", ws_pkt.payload[i]);
            }

            Protocol::Request req_pkt;
            req_pkt.id = (ws_pkt.payload[0] << 8) | ws_pkt.payload[1];
            req_pkt.cmd = ws_pkt.payload[2];
            uint8_t header_size = sizeof(req_pkt.id) + sizeof(req_pkt.cmd);
            req_pkt.len = ws_pkt.len - header_size;
            req_pkt.payload = &ws_pkt.payload[header_size];

            // Store the pending request
            bool stored = false;
            for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_CALLS; ++i) {
                if (pending_requests[i].id == 0) {
                    pending_requests[i].id = req_pkt.id;
                    pending_requests[i].req = req;
                    stored = true;
                    break;
                }
            }
            if (!stored) {
                Log::Add(Log::Level::Error, "No space to store pending request ID: %d", req_pkt.id);
                free(ws_pkt.payload);
                return ESP_ERR_NO_MEM;
            }

            Error err = Protocol::Handle(req_pkt, [](const Protocol::Response& res) {
                httpd_req_t* req;
                if (FindPendingRequest(res.id, &req) != Error::Ok) {
                    Log::Add(Log::Level::Warning, "No pending request found for response ID: %d", res.id);
                    return;
                }

                httpd_ws_frame_t ws_res;
                memset(&ws_res, 0, sizeof(httpd_ws_frame_t));
                ws_res.payload = (uint8_t*) malloc(sizeof(res.id) + sizeof(res.status) + sizeof(res.len) + res.len);
                if (!ws_res.payload) {
                    Log::Add(Log::Level::Error, "Memory allocation failed for response");
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
                    Log::Add(Log::Level::Error, "WebSocket send error: %d", ret);
                }
                free(ws_res.payload);
            });

            if (err != Error::Ok) {
                Log::Add(Log::Level::Error, "Protocol::Handle error: %d", static_cast<uint8_t>(err));
                ret = ESP_FAIL;
            }
        }

        free(ws_pkt.payload);
        return ret;
    }

    void on_connected(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        Log::Add(Log::Level::Debug, "WebSocket client connected");
    }

    void on_disconnected(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        Log::Add(Log::Level::Debug, "WebSocket client disconnected");
    }

    Error Init()
    {
        // nothing to do here
        return Error::Ok;
    }

    Error Attach()
    {
        httpd_uri_t ws = {
            .uri        = "/",
            .method     = HTTP_GET,
            .handler    = ws_handler,
            .user_ctx   = NULL,
            .is_websocket = true,
            .handle_ws_control_frames = false,
            .supported_subprotocol = ""
        };

        httpd_register_uri_handler((httpd_handle_t) WebServer::_get_server_handle(), &ws);
        esp_event_handler_register(ESP_HTTP_SERVER_EVENT, HTTP_SERVER_EVENT_ON_CONNECTED, on_connected, nullptr);
        esp_event_handler_register(ESP_HTTP_SERVER_EVENT, HTTP_SERVER_EVENT_DISCONNECTED, on_disconnected, nullptr);

        return Error::Ok;
    }
}