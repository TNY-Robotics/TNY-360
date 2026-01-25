#include "network/WebInterface.hpp"
#include "common/Log.hpp"
#include "common/LittleFS.hpp"
#include "locomotion/MotorController.hpp"
#include "Robot.hpp"

esp_err_t WebInterface::interface_handler(httpd_req_t *req)
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

esp_err_t WebInterface::api_position_handler(httpd_req_t *req)
{
    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    char *comma = strchr(buf, ',');
    if (comma) {
        // Retrieve the number after the comma
        int value = atoi(comma + 1);
        
        // Optional: Retrieve the ID (the number before the comma)
        int id = atoi(buf);

        Joint* legs[12] = {
            &Robot::GetInstance().getBody().getFrontLeftLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getFrontLeftLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getFrontLeftLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getBackRightLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getBackRightLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getBackRightLeg().getKneePitch(),
        };
        
        Log::Add(Log::Level::Debug, TAG, "Received position value: %d", value);
        if (id >= 0 && id < 12) {
            float angle = static_cast<float>(value); // assuming value is in range 0-1000
            legs[id]->setTarget(DEG_TO_RAD(angle));
            Log::Add(Log::Level::Debug, TAG, "Set leg ID %d target angle to %.3f", id, angle);
        } else {
            Log::Add(Log::Level::Error, TAG, "Invalid leg ID: %d", id);
        }
    }

    // respond
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


esp_err_t WebInterface::api_enable_handler(httpd_req_t *req)
{
    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    char *comma = strchr(buf, ',');
    if (comma) {
        // Retrieve the boolean after the comma (0 for disable, 1 for enable)
        int enable = atoi(comma + 1);
        
        // Retrieve the ID (the number before the comma)
        int id = atoi(buf); 

        Log::Add(Log::Level::Debug, TAG, "Received enable value: %d for ID: %d", enable, id);

        Joint* legs[12] = {
            &Robot::GetInstance().getBody().getFrontLeftLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getFrontLeftLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getFrontLeftLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getBackRightLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getBackRightLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getBackRightLeg().getKneePitch(),
        };

        if (id >= 0 && id < 12) {
            if (enable) {
                legs[id]->enable();
                Log::Add(Log::Level::Debug, TAG, "Enabled leg ID %d", id);
            } else {
                legs[id]->disable();
                Log::Add(Log::Level::Debug, TAG, "Disabled leg ID %d", id);
            }
        } else {
            Log::Add(Log::Level::Error, TAG, "Invalid leg ID: %d", id);
        }
    }

    // respond
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WebInterface::api_feedback_handler(httpd_req_t *req)
{
    Joint* legs[12] = {
        &Robot::GetInstance().getBody().getFrontLeftLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getFrontLeftLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getFrontLeftLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getBackRightLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getBackRightLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getBackRightLeg().getKneePitch(),
    };

    char response[256];
    int offset = 0;
    for (int i = 0; i < 12; ++i) {
        float angle = 0.0f;
        legs[i]->getFeedback(angle);
        offset += snprintf(response + offset, sizeof(response) - offset, "%.1f,", RAD_TO_DEG(angle));
    }

    // respond
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, response, offset - 1); // exclude last comma

    return ESP_OK;
}

esp_err_t WebInterface::api_voltages_handler(httpd_req_t *req)
{
    Joint* legs[12] = {
        &Robot::GetInstance().getBody().getFrontLeftLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getFrontLeftLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getFrontLeftLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getBackRightLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getBackRightLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getBackRightLeg().getKneePitch(),
    };

    char response[256];
    int offset = 0;
    for (int i = 0; i < 12; ++i) {
        AnalogDriver::Value voltage = 0;
        AnalogDriver::GetVoltage(legs[i]->getMotorController().getAnalogChannel(), &voltage);
        offset += snprintf(response + offset, sizeof(response) - offset, "%d,", voltage);
    }

    // respond
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, response, offset - 1); // exclude last comma

    return ESP_OK;
}

esp_err_t WebInterface::api_calibrate_handler(httpd_req_t *req)
{
    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
        
    // Retrieve the ID (the number before the comma)
    int id = atoi(buf);

    if (id == 12) {
        Log::Add(Log::Level::Info, TAG, "Starting full body calibration");
        Robot::GetInstance().getBody().startCalibration();
        // respond
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    Joint* legs[12] = {
        &Robot::GetInstance().getBody().getFrontLeftLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getFrontLeftLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getFrontLeftLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getFrontRightLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getBackLeftLeg().getKneePitch(),
        &Robot::GetInstance().getBody().getBackRightLeg().getHipRoll(),
        &Robot::GetInstance().getBody().getBackRightLeg().getHipPitch(),
        &Robot::GetInstance().getBody().getBackRightLeg().getKneePitch(),
    };

    if (id >= 0 && id < 12) {
        Log::Add(Log::Level::Info, TAG, "Starting calibration for leg ID %d", id);
        if (Error err = legs[id]->getMotorController().startCalibration(); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to start calibration for leg ID %d", id);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
    } else {
        Log::Add(Log::Level::Error, TAG, "Invalid leg ID: %d", id);
    }

    // respond
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "ok", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WebInterface::api_ik_handler(httpd_req_t* req)
{
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Parse input for leg index and target coordinates
    int leg_index;
    float x, y, z;
    if (sscanf(buf, "%d,%f,%f,%f", &leg_index, &x, &y, &z) != 4) {
        Log::Add(Log::Level::Error, TAG, "Invalid IK input format");
        httpd_resp_send(req, "invalid input", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    Leg* legs[4] = {
        &Robot::GetInstance().getBody().getFrontLeftLeg(),
        &Robot::GetInstance().getBody().getFrontRightLeg(),
        &Robot::GetInstance().getBody().getBackLeftLeg(),
        &Robot::GetInstance().getBody().getBackRightLeg(),
    };

    if (leg_index < 0 || leg_index >= 4) {
        Log::Add(Log::Level::Error, TAG, "Invalid leg index: %d", leg_index);
        httpd_resp_send(req, "invalid leg index", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    Vec3f target_pos = {x, y, z};
    if (Error err = legs[leg_index]->setTarget(target_pos); err != Error::None) {
        Log::Add(Log::Level::Error, TAG, "IK computation failed for leg index %d", leg_index);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_send(req, "ik ok", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WebInterface::api_body_handler(httpd_req_t* req)
{
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Parse input for body posture
    float rotx, roty, rotz, posx, posy, posz;
    if (sscanf(buf, "%f,%f,%f,%f,%f,%f", &rotx, &roty, &rotz, &posx, &posy, &posz) != 6) {
        Log::Add(Log::Level::Error, TAG, "Invalid Body Posture input format");
        httpd_resp_send(req, "invalid input", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    Log::Add(Log::Level::Debug, TAG, "Setting body posture to pos(%.2f, %.2f, %.2f) rot(%.2f, %.2f, %.2f)", posx, posy, posz, rotx, roty, rotz);
    
    Transformf posture(
        Vec3f(posx, posy, posz),
        Quatf::FromEulerAngles(Vec3f(DEG_TO_RAD(rotx), DEG_TO_RAD(roty), DEG_TO_RAD(rotz)))
    );
    if (Error err = Robot::GetInstance().getBody().setPosture(posture); err != Error::None) {
        Log::Add(Log::Level::Error, TAG, "Setting body posture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_send(req, "body posture ok", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WebInterface::api_connect_handler(httpd_req_t *req)
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

    Log::Add(Log::Level::Info, TAG, "Connecting to AP: SSID=%s, Password=%s", ssid, password);
    if (Error err = Robot::GetInstance().getNetworkManager().getWiFiManager().connect(ssid, password); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to connect to AP");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_send(req, "connecting...", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

WebInterface::WebInterface(uint16_t web_port)
    : port(web_port)
{
}

Error WebInterface::init()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_uri_handlers = 8;
    config.ctrl_port = port + 1;
    config.max_uri_handlers = 12; // NOTE: Increase if more handlers are added

    if (httpd_start(&server, &config) != ESP_OK)
    {
        server = nullptr;
        Log::Add(Log::Level::Error, TAG, "Failed to start web server");
        return Error::Unknown;
    }
    running = true;

    // load content
    if (LittleFS::LoadFileContent("interface.html", &interface_file_content, nullptr) != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to load web interface content");
        return Error::Unknown;
    }
    Log::Add(Log::Level::Debug, TAG, "Web interface content loaded");

    registerURIHandlers();
    return Error::None;
}

Error WebInterface::deinit()
{
    if (server)
    {
        httpd_stop(server);
        server = nullptr;
    }
    running = false;
    return Error::None;
}

void WebInterface::registerURIHandlers()
{
    httpd_uri_t interface_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->interface_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &interface_uri);

    httpd_uri_t api_set_uri = {
        .uri       = "/api/position",
        .method    = HTTP_POST,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_position_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_set_uri);

    httpd_uri_t api_get_uri = {
        .uri       = "/api/enable",
        .method    = HTTP_POST,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_enable_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_get_uri);

    httpd_uri_t api_feedback_uri = {
        .uri       = "/api/feedback",
        .method    = HTTP_GET,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_feedback_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_feedback_uri);

    httpd_uri_t api_voltages_uri = {
        .uri       = "/api/voltages",
        .method    = HTTP_GET,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_voltages_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_voltages_uri);

    httpd_uri_t api_calibrate_uri = {
        .uri       = "/api/calibrate",
        .method    = HTTP_POST,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_calibrate_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_calibrate_uri);

    httpd_uri_t api_ik_uri = {
        .uri       = "/api/ik",
        .method    = HTTP_POST,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_ik_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_ik_uri);

    httpd_uri_t api_body_uri = {
        .uri       = "/api/body",
        .method    = HTTP_POST,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_body_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_body_uri);

    httpd_uri_t api_connect_uri = {
        .uri       = "/api/connect",
        .method    = HTTP_POST,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            WebInterface* web_interface = static_cast<WebInterface*>(req->user_ctx);
            return web_interface->api_connect_handler(req);
        },
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &api_connect_uri);

    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, [](httpd_req_t *req, httpd_err_code_t error) -> esp_err_t {
        // Set status
        httpd_resp_set_status(req, "302 Temporary Redirect");
        // Redirect to the "/" root directory
        httpd_resp_set_hdr(req, "Location", "/");
        // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
        httpd_resp_send(req, "Redirect to the web interface", HTTPD_RESP_USE_STRLEN);

        Log::Add(Log::Level::Debug, TAG, "Redirecting to web interface");
        return ESP_OK;
    });
}