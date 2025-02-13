#include "websocket.hpp"
#include <esp_log.h>
#include <esp_http_server.h>

#define TAG "websocket"
#define SERVER_PORT 5261

esp_err_t ws_handler(httpd_req_t *req);

httpd_handle_t server = NULL;
httpd_uri_t ws = {
    .uri        = "/",
    .method     = HTTP_GET,
    .handler    = ws_handler,
    .user_ctx   = NULL,
    .is_websocket = true
};

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Nouvelle connexion WebSocket");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = NULL;

    // Récupérer la longueur du message
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur de réception du message");
        return ret;
    }

    ws_pkt.payload = (uint8_t*) malloc(ws_pkt.len + 1);
    if (!ws_pkt.payload) {
        ESP_LOGE(TAG, "Allocation mémoire échouée");
        return ESP_ERR_NO_MEM;
    }

    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret == ESP_OK) {
        ws_pkt.payload[ws_pkt.len] = 0;  // Null-terminate pour éviter les erreurs
        ESP_LOGI(TAG, "Message reçu : %s", (char *)ws_pkt.payload);

        // Répondre au client
        httpd_ws_frame_t ws_resp = {
            .type = HTTPD_WS_TYPE_TEXT,
            .payload = (uint8_t *)"Message reçu",
            .len = strlen("Message reçu")
        };
        httpd_ws_send_frame(req, &ws_resp);
    }

    free(ws_pkt.payload);
    return ret;
}

esp_err_t websocket_stop()
{
    // Stop the httpd server
    ESP_LOGI(TAG, "Stopping server");
    if (server == NULL) {
        return ESP_OK;
    }
    return httpd_stop(server);
}

esp_err_t websocket_start()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // config.server_port = SERVER_PORT;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);

        return ESP_OK;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return ESP_FAIL;
}
