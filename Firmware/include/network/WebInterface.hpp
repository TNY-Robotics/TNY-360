#pragma once
#include "common/utils.hpp"
#include "esp_http_server.h"

class WebInterface
{
public:
    constexpr static const char* TAG = "WebInterface";
    constexpr static const char* MOUNT_POINT = "/storage/web";

    WebInterface(uint16_t web_port = 80);

    /**
     * @brief Initialize the web interface.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the web interface.
     * @return Error code indicating success or failure.
     */
    Error deinit();

private:
    httpd_handle_t server = nullptr;
    bool running = false;
    const uint16_t port;

    void registerURIHandlers();

    static const char* get_mime_type(const char* filepath);
    static esp_err_t send_file_chunked(httpd_req_t *req, const char *filepath, const char *mime_type, bool is_gzip);
    static esp_err_t main_request_handler(httpd_req_t *req);
};