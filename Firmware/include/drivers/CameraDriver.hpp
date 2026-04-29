#pragma once
#include "common/utils.hpp"
#include "esp_camera.h"
#include "esp_http_server.h"

class CameraDriver
{
public:
    CameraDriver();
    ~CameraDriver();

    Error init();
    Error deinit();

    Error start();
    Error stop();

private:
    sensor_t* sensor;
    httpd_handle_t server;
};