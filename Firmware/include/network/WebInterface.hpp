#pragma once
#include "common/utils.hpp"
#include "esp_http_server.h"

class WebInterface
{
public:
    constexpr static const char* TAG = "WebInterface";

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
    char* interface_file_content = nullptr;

    void registerURIHandlers();
    esp_err_t interface_handler(httpd_req_t *req);
    esp_err_t api_position_handler(httpd_req_t *req);
    esp_err_t api_enable_handler(httpd_req_t *req);
    esp_err_t api_feedback_handler(httpd_req_t *req);
    esp_err_t api_voltages_handler(httpd_req_t *req);
    esp_err_t api_calibrate_handler(httpd_req_t *req);
    esp_err_t api_ik_handler(httpd_req_t* req);
    esp_err_t api_body_handler(httpd_req_t *req);
    esp_err_t api_connect_handler(httpd_req_t *req);
};