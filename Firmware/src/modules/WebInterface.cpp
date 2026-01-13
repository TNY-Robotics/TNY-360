#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstring>
#include "modules/Motor.hpp"
#include "modules/AnalogScanner.hpp"
#include "modules/WebInterface.hpp"
#include "modules/LittleFS.hpp"
#include "modules/Log.hpp"
#include "esp_http_server.h"

namespace WebInterface
{
    // Static variables
    httpd_handle_t server = nullptr;
    bool running = false;

    char* interface_file_content = nullptr; // Will hold the content of interface.html

    esp_err_t interface_handler(httpd_req_t *req)
    {
        httpd_resp_set_type(req, "text/html");
        if (interface_file_content)
        {
            httpd_resp_send(req, interface_file_content, HTTPD_RESP_USE_STRLEN);
        } else
        {
            httpd_resp_send(req, "interface.html not found", HTTPD_RESP_USE_STRLEN);
        }
        return ESP_OK;
    }

    esp_err_t api_motor_handler(httpd_req_t *req)
    {
        // Handle motor control API
        char buf[128] = {0};
        int received = httpd_req_recv(req, buf, sizeof(buf) - 1);
        Log::Add(Log::Level::Debug, "API Motor Handler received %s (%d)", buf, received);
        if (received <= 0) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        // Simple parsing, expecting format: ?index,position,force
        int index = 0, position = 0, force = 0;
        if (sscanf(buf, "%d,%d,%d", &index, &position, &force) == 3) {
            Log::Add(Log::Level::Info, "API: Set motor %d to position %d with force %d", index, position, force);
            // Here you would add code to actually control the motor
            Motor::SetForce(static_cast<Motor::Id>(index), static_cast<Motor::ForceLevel>(force));
            Motor::SetAngle(static_cast<Motor::Id>(index), static_cast<float>(position));
        } else {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        // respond something
        httpd_resp_set_type(req, "application/json");
        const char* resp_str = "{\"status\":\"ok\"}";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    esp_err_t api_motor_pwm_handler(httpd_req_t *req)
    {
        // Handle motor PWM control API
        char buf[128] = {0};
        int received = httpd_req_recv(req, buf, sizeof(buf) - 1);
        Log::Add(Log::Level::Debug, "API Motor PWM Handler received %s (%d)", buf, received);
        if (received <= 0) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        // Simple parsing, expecting format: ?index,pwm_value
        int index = 0, pwm_value = 0;
        if (sscanf(buf, "%d,%d", &index, &pwm_value) == 2) {
            Log::Add(Log::Level::Info, "API: Set motor %d to PWM value %d", index, pwm_value);
            // Here you would add code to actually control the motor PWM
            Motor::SetPWM(static_cast<Motor::Id>(index), static_cast<uint16_t>(pwm_value));
        } else {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        // respond something
        httpd_resp_set_type(req, "application/json");
        const char* resp_str = "{\"status\":\"ok\"}";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    esp_err_t api_angle_handler(httpd_req_t *req)
    {
        // return all motor angles and raw values
        float angles[static_cast<int>(Motor::Id::Count)];
        Motor::GetCurrentAngles(angles);

        char response[256];
        int len = 0;
        for (int i = 0; i < static_cast<int>(Motor::Id::Count); i++) {
            len += snprintf(response + len, sizeof(response) - len, "%.2f%s", angles[i], (i < static_cast<int>(Motor::Id::Count) - 1) ? "," : "");
        }

        // send response
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send(req, response, len);
        return ESP_OK;
    }

    esp_err_t api_voltage_handler(httpd_req_t *req)
    {
        // return all motor angles and raw values
        int voltages[static_cast<int>(AnalogScanner::Id::Count)];
        AnalogScanner::GetRawValues(voltages);

        char response[256];
        int len = 0;
        for (int i = 0; i < static_cast<int>(AnalogScanner::Id::Count); i++) {
            len += snprintf(response + len, sizeof(response) - len, "%d%s", voltages[i], (i < static_cast<int>(AnalogScanner::Id::Count) - 1) ? "," : "");
        }

        // send response
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send(req, response, len);
        return ESP_OK;
    }

    Error Init()
    {
        // Nothing to do here for now
        return Error::Ok;
    }

    Error Start(int port)
    {
        if (running) {
            return Error::Ok; // Already running
        }

        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.server_port = port;
        config.max_uri_handlers = 8;
        config.ctrl_port = port + 1;

        if (httpd_start(&server, &config) != ESP_OK)
        {
            server = nullptr;
            return Error::Unknown;
        }
        running = true;

        // load content
        if (LittleFS::LoadFileContent("interface.html", &interface_file_content, nullptr) != Error::Ok)
        {
            Log::Add(Log::Level::Error, "Failed to load web interface content");
            return Error::Unknown;
        }
        Log::Add(Log::Level::Debug, "Web interface content loaded");

        // attach web interface handler
        httpd_uri_t interface_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = interface_handler,
            .user_ctx  = nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        
        if (httpd_register_uri_handler((httpd_handle_t) server, &interface_uri) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Failed to register web interface URI handler");
            return Error::Unknown;
        }
        
        // web interface motor API handler
        httpd_uri_t api_connect_uri = {
            .uri       = "/api/motor",
            .method    = HTTP_POST,
            .handler   = api_motor_handler,
            .user_ctx  = nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };

        if (httpd_register_uri_handler((httpd_handle_t) server, &api_connect_uri) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Failed to register web interface API URI handler");
            return Error::Unknown;
        }

        // web interface angle API handler
        httpd_uri_t api_angle_uri = {
            .uri       = "/api/angle",
            .method    = HTTP_GET,
            .handler   = api_angle_handler,
            .user_ctx  = nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };

        if (httpd_register_uri_handler((httpd_handle_t) server, &api_angle_uri) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Failed to register web interface angle API URI handler");
            return Error::Unknown;
        }

        // web interface voltage API handler
        httpd_uri_t api_voltage_uri = {
            .uri       = "/api/voltage",
            .method    = HTTP_GET,
            .handler   = api_voltage_handler,
            .user_ctx  = nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };

        if (httpd_register_uri_handler((httpd_handle_t) server, &api_voltage_uri) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Failed to register web interface voltage API URI handler");
            return Error::Unknown;
        }

        // web interface motor PWM API handler
        httpd_uri_t api_motor_pwm_uri = {
            .uri       = "/api/motor_pwm",
            .method    = HTTP_POST,
            .handler   = api_motor_pwm_handler,
            .user_ctx  = nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };

        if (httpd_register_uri_handler((httpd_handle_t) server, &api_motor_pwm_uri) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Failed to register web interface motor PWM API URI handler");
            return Error::Unknown;
        }

        return Error::Ok;
    }

    void Stop()
    {
        if (running && server)
        {
            httpd_stop(server);
            server = nullptr;
            running = false;
        }
    }

    bool IsRunning()
    {
        return running;
    }

    void* _get_server_handle() // For internal use
    {
        return (void*)server;
    }
}