#include "network/WebInterface.hpp"
#include <string>
#include "common/Log.hpp"
#include "common/LittleFS.hpp"
#include "locomotion/MotorController.hpp"
#include "ArduinoJson.hpp"
#include "Robot.hpp"

// Code-embeded safemode html page
extern const uint8_t safemode_html_start[] asm("_binary_safemode_html_start");
extern const uint8_t safemode_html_end[]   asm("_binary_safemode_html_end");


WebInterface::WebInterface(uint16_t web_port)
    : port(web_port)
{
}

Error WebInterface::init()
{
    LOG_SCOPE(TAG, "WebInterface::init");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_uri_handlers = 8;
    config.ctrl_port = port + 1;
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_open_sockets = 6;
    config.lru_purge_enable = true;
    config.recv_wait_timeout = 5;
    config.send_wait_timeout = 5;

    LOG_DEBUG(TAG, "Starting web server on port %d", port);
    if (httpd_start(&server, &config) != ESP_OK)
    {
        server = nullptr;
        LOG_ERROR(TAG, "Failed to start web server");
        ErrorHandle(ErrorStruct::WebInterfaceInitFailed);
        return Error::Unknown;
    }
    running = true;

    // initialize storage
    if (Error err = LittleFS::Init(); err != Error::None)
    {
        return err;
    }

    // check if website files are present
    bool files_present = false;
    FILE* fd = fopen((std::string(MOUNT_POINT) + "/index.html").c_str(), "r");
    if (!fd) {
        fd = fopen((std::string(MOUNT_POINT) + "/index.html.gz").c_str(), "r");
    }
    if (fd) {
        files_present = true;
        fclose(fd);
    }
    if (!files_present)
    {
        LOG_ERROR(TAG, "Website files not found in LittleFS. Switching to safe mode.");

        // Register safe-mode handlers (index page + connect api)
        httpd_uri_t safe_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = safe_request_handler,
            .user_ctx  = this,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr,
        };
        httpd_register_uri_handler(server, &safe_uri);
        httpd_uri_t connect_uri = {
            .uri       = "/connect",
            .method    = HTTP_POST,
            .handler   = connect_request_handler,
            .user_ctx  = this,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr,
        };
        httpd_register_uri_handler(server, &connect_uri);
        // redirection to web page for captive portal
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, [](httpd_req_t *req, httpd_err_code_t error) -> esp_err_t {
            // return to homepage on 404 to let the SPA handle it + Captive portal support
            char redirect_url[64];
            snprintf(redirect_url, sizeof(redirect_url), "http://%s/", Robot::GetInstance().getNetworkManager().getWiFiManager().getIPAddr());

            httpd_resp_set_status(req, "302 Temporary Redirect");
            httpd_resp_set_hdr(req, "Location", redirect_url);
            httpd_resp_send(req, "Redirecting...", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        });
        return Error::None;
    }

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
    httpd_uri_t catch_all_uri = {
        .uri       = "/*", // Wildcard
        .method    = HTTP_GET,
        .handler   = main_request_handler,
        .user_ctx  = this,
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
    };
    httpd_register_uri_handler(server, &catch_all_uri);

    // Note: The 404 handler is less useful here because "/*" catches everything, 
    // except if the method is not GET (e.g., POST/PUT)
    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, [](httpd_req_t *req, httpd_err_code_t error) -> esp_err_t {
        // return to homepage on 404 to let the SPA handle it + Captive portal support
        char redirect_url[64];
        snprintf(redirect_url, sizeof(redirect_url), "http://%s/", Robot::GetInstance().getNetworkManager().getWiFiManager().getIPAddr());

        httpd_resp_set_status(req, "302 Temporary Redirect");
        httpd_resp_set_hdr(req, "Location", redirect_url);
        httpd_resp_send(req, "Redirecting...", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    });
}

const char* WebInterface::get_mime_type(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (!ext) return "application/octet-stream";
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".js") == 0)   return "application/javascript";
    if (strcmp(ext, ".css") == 0)  return "text/css";
    if (strcmp(ext, ".png") == 0)  return "image/png";
    if (strcmp(ext, ".jpg") == 0)  return "image/jpeg";
    if (strcmp(ext, ".ico") == 0)  return "image/x-icon";
    if (strcmp(ext, ".svg") == 0)  return "image/svg+xml";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".woff2") == 0) return "font/woff2";
    if (strcmp(ext, ".glb") == 0)  return "model/gltf-binary";

    return "text/plain";
}

esp_err_t WebInterface::send_file_chunked(httpd_req_t *req, const char *filepath, const char *mime_type, bool is_gzip)
{
    FILE *fd = fopen(filepath, "r");
    if (!fd)
    {
        LOG_ERROR(TAG, "Failed to read file: %s", filepath);
        return HTTPD_500_INTERNAL_SERVER_ERROR;
    }

    httpd_resp_set_type(req, mime_type);
    if (is_gzip)
    {
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    }

    char *chunk = (char*)malloc(4096); // 4KB buffer
    if (!chunk)
    {
        fclose(fd);
        return HTTPD_500_INTERNAL_SERVER_ERROR;
    }

    size_t chunksize;
    do
    {
        chunksize = fread(chunk, 1, 4096, fd);
        if (chunksize > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK)
            {
                fclose(fd);
                free(chunk);
                return ESP_FAIL;
            }
        }
    } while (chunksize != 0);

    fclose(fd);
    free(chunk);
    
    // Indicate the end of the response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t WebInterface::main_request_handler(httpd_req_t *req)
{
    // Check host type to detect captive portal requests
    char host_header[64];
    char my_ip[16];
    strncpy(my_ip, Robot::GetInstance().getNetworkManager().getWiFiManager().getIPAddr(), sizeof(my_ip) - 1);
    if (httpd_req_get_hdr_value_str(req, "Host", host_header, sizeof(host_header)) == ESP_OK)
    {
        if (strstr(host_header, my_ip) == NULL && strstr(host_header, "localhost") == NULL)
        {
            char redirect_url[64];
            snprintf(redirect_url, sizeof(redirect_url), "http://%s/", my_ip);
            httpd_resp_set_status(req, "302 Found");
            httpd_resp_set_hdr(req, "Location", redirect_url);
            httpd_resp_send(req, "Redirecting to dashboard...", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
    }

    std::string filepath = MOUNT_POINT;
    filepath += req->uri;

    // Cleaning the URI by removing query parameters (if any)
    size_t query_pos = filepath.find('?');
    if (query_pos != std::string::npos) {
        filepath = filepath.substr(0, query_pos);
    }

    struct stat st;

    // Checking for a gzipped version of the file first (e.g., index.html.gz for index.html)
    std::string filepath_gz = filepath + ".gz";
    if (stat(filepath_gz.c_str(), &st) == 0)
    {
        return send_file_chunked(req, filepath_gz.c_str(), get_mime_type(filepath.c_str()), true);
    }

    // Checking if the file/folder exists (otherwise fallback to SPA)
    if (stat(filepath.c_str(), &st) != 0)
    {        
        std::string index_path = std::string(MOUNT_POINT) + "/index.html.gz";
        if (stat(index_path.c_str(), &st) == 0) {
            return send_file_chunked(req, index_path.c_str(), "text/html", true);
        }

        return httpd_resp_send_404(req);
    }

    // If it's a directory, try to serve index.html.gz or index.html
    if (S_ISDIR(st.st_mode))
    {
        std::string index_gz_path = filepath + "/index.html.gz";
        if (stat(index_gz_path.c_str(), &st) == 0) {
            return send_file_chunked(req, index_gz_path.c_str(), "text/html", true);
        }

        std::string index_path = filepath + "/index.html";
        if (stat(index_path.c_str(), &st) == 0) {
            return send_file_chunked(req, index_path.c_str(), "text/html", false);
        }

        return httpd_resp_send_404(req);
    }
    
    // Last option : serve the file directly
    return send_file_chunked(req, filepath.c_str(), get_mime_type(filepath.c_str()), false);
}

esp_err_t WebInterface::safe_request_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char*)safemode_html_start, safemode_html_end - safemode_html_start);
}

esp_err_t WebInterface::connect_request_handler(httpd_req_t *req)
{
    // Parse the body as JSON to extract WiFi credentials
    char buffer[512];

    if (req->content_len >= sizeof(buffer))
    {
        return httpd_resp_send(req, "Payload too large", HTTPD_RESP_USE_STRLEN);
    }
    
    if (httpd_req_recv(req, buffer, req->content_len) <= 0)
    {
        return httpd_resp_send(req, "Failed to receive data", HTTPD_RESP_USE_STRLEN);
    }

    ArduinoJson::JsonDocument json;

    ArduinoJson::DeserializationError err = ArduinoJson::deserializeJson(json, buffer);
    if (err)
    {
        return httpd_resp_send(req, "Invalid JSON", HTTPD_RESP_USE_STRLEN);
    }

    const char* ssid = json["ssid"];
    const char* password = json["password"];

    if (!ssid || !password)
    {
        return httpd_resp_send(req, "Missing ssid or password", HTTPD_RESP_USE_STRLEN);
    }

    // Attempt to connect to the new WiFi network
    Error wifi_err = Robot::GetInstance().getNetworkManager().getWiFiManager().connect(ssid, password);
    if (wifi_err != Error::None)
    {
        return httpd_resp_send(req, "Failed to connect to WiFi", HTTPD_RESP_USE_STRLEN);
    }

    return httpd_resp_send(req, "Connecting to WiFi...", HTTPD_RESP_USE_STRLEN);
}