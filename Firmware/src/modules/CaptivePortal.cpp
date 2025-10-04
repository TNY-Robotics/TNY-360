#include "modules/CaptivePortal.hpp"
#include "modules/WebServer.hpp"
#include "modules/LittleFS.hpp"
#include "modules/Log.hpp"
#include "modules/WiFi.hpp"
#include "modules/Network.hpp"
#include "esp_http_server.h"

namespace CaptivePortal
{
    static char* index_file_content = nullptr; // Will hold the content of index.html

    static esp_err_t captive_portal_handler(httpd_req_t *req)
    {
        httpd_resp_set_type(req, "text/html");
        if (index_file_content)
        {
            httpd_resp_send(req, index_file_content, HTTPD_RESP_USE_STRLEN);
        } else
        {
            httpd_resp_send(req, "index.html not found", HTTPD_RESP_USE_STRLEN);
        }
        return ESP_OK;
    }

    static esp_err_t api_connect_handler(httpd_req_t *req)
    {
        // NOTE : Those malloc should be freed after use, but since we are going to restart the ESP32, it's not a big deal.
        char* ssid = (char*) malloc(64 * sizeof(char));
        char* password = (char*) malloc(64 * sizeof(char));

        // Get content length
        char buf[256] = {0};
        int received = httpd_req_recv(req, buf, sizeof(buf) - 1);
        if (received <= 0) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        // Parse form data (simple parser for application/x-www-form-urlencoded)
        char *ssid_ptr = strstr(buf, "ssid=");
        char *pass_ptr = strstr(buf, "password=");
        if (ssid_ptr) {
            ssid_ptr += 5;
            char *end = strchr(ssid_ptr, '&');
            size_t len = end ? (size_t)(end - ssid_ptr) : strlen(ssid_ptr);
            strncpy(ssid, ssid_ptr, len);
            ssid[len] = '\0';
        }
        if (pass_ptr) {
            pass_ptr += 9;
            char *end = strchr(pass_ptr, '&');
            size_t len = end ? (size_t)(end - pass_ptr) : strlen(pass_ptr);
            strncpy(password, pass_ptr, len);
            password[len] = '\0';
        }

        Log::Add(Log::Level::Info, "Connecting to AP: SSID=%s, Password=%s", ssid, password);
        WiFi::Connect(ssid, password,
            []() {
                Log::Add(Log::Level::Info, "Connected to Wi-Fi successfully, rebooting in 1 second...");
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            },
            [](Error err) {
                Log::Add(Log::Level::Error, "Failed to connect to Wi-Fi, starting up in AP mode again...", static_cast<uint8_t>(err));
                WiFi::StartAP(WIFI_AP_SSID, WIFI_AP_PASSWORD,
                    []() {
                        Log::Add(Log::Level::Info, "AP started successfully");
                        if (Network::SetupAP() != Error::Ok) {
                            Log::Add(Log::Level::Error, "Network SetupAP failed");
                            return;
                        }
                    },
                    [](Error err) {
                        Log::Add(Log::Level::Error, "Failed to start AP, this might be a serious issue.");
                    }
                );
            }
        );

        httpd_resp_send(req, "connecting...", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
    {
        // Set status
        httpd_resp_set_status(req, "302 Temporary Redirect");
        // Redirect to the "/" root directory
        httpd_resp_set_hdr(req, "Location", "/");
        // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
        httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

        Log::Add(Log::Level::Debug, "Redirecting to captive portal");
        return ESP_OK;
    }

    Error Init()
    {
        if (LittleFS::LoadFileContent("index.html", &index_file_content, nullptr) != Error::Ok)
        {
            Log::Add(Log::Level::Error, "Failed to load captive portal content");
            return Error::Unknown;
        }
        Log::Add(Log::Level::Debug, "Captive portal content loaded");
        return Error::Ok;
    }

    Error Attach()
    {
        httpd_uri_t captive_portal_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = captive_portal_handler,
            .user_ctx  = nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };

        httpd_uri_t api_connect_uri = {
            .uri       = "/api/connect",
            .method    = HTTP_POST,
            .handler   = api_connect_handler,
            .user_ctx  = nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };

        httpd_register_uri_handler((httpd_handle_t) WebServer::_get_server_handle(), &captive_portal_uri);
        httpd_register_uri_handler((httpd_handle_t) WebServer::_get_server_handle(), &api_connect_uri);
        httpd_register_err_handler((httpd_handle_t) WebServer::_get_server_handle(), HTTPD_404_NOT_FOUND, http_404_error_handler);

        return Error::Ok;
    }
}