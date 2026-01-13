#include <cstring>
#include <cstdio>
#include <cmath>
#include "modules/WebServer.hpp"
#include "modules/Log.hpp"
#include "esp_http_server.h"

namespace WebServer
{
    // Static variables
    httpd_handle_t server = nullptr;
    bool running = false;
\
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