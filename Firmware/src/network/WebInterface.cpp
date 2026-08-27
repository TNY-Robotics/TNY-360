#include "network/WebInterface.hpp"
#include <string>
#include "common/Log.hpp"
#include "common/LittleFS.hpp"
#include "locomotion/MotorController.hpp"
#include "ArduinoJson.hpp"
#include "Robot.hpp"
#include <sys/stat.h>

// Code-embeded safemode html page
extern const uint8_t safemode_html_start[] asm("_binary_safemode_html_start");
extern const uint8_t safemode_html_end[]   asm("_binary_safemode_html_end");

void sanitize_filepath(char* path) {
    if (!path) return;
    
    int write_idx = 0;
    for (int read_idx = 0; path[read_idx] != '\0'; ++read_idx) {
        char c = path[read_idx];
        
        if ((c >= 'a' && c <= 'z') || 
            (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || 
            c == '.' || c == '-' || c == '_' || c == '/') {
            
            path[write_idx++] = c;
        } 
        else if (c == ' ') {
            path[write_idx++] = '_';
        }
    }
    
    path[write_idx] = '\0';
}

WebInterface::WebInterface(WiFiManager* wifi_manager, uint16_t web_port)
    : wifi_manager(wifi_manager), port(web_port)
{
}

Status WebInterface::init()
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
    config.stack_size = 8192; // 8KB stack size instead of 4 for the httpd task

    LOG_DEBUG(TAG, "Starting web server on port %d", port);
    if (httpd_start(&server, &config) != ESP_OK)
    {
        server = nullptr;
        LOG_ERROR(TAG, "Failed to start web server");
        // ErrorHandle(ErrorStruct::WebInterfaceInitFailed);
        return Status::Unknown;
    }
    running = true;

    // initialize storage
    if (Status err = LittleFS::Init(); err != Status::Ok)
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
        return Status::Ok;
    }

    registerURIHandlers();
    return Status::Ok;
}

Status WebInterface::deinit()
{
    if (server)
    {
        httpd_stop(server);
        server = nullptr;
    }
    running = false;
    return Status::Ok;
}

void WebInterface::registerURIHandlers()
{
    httpd_uri_t catch_all_uri = {
        .uri       = "/*", // Wildcard
        .method    = HTTP_GET,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            auto self = static_cast<WebInterface*>(req->user_ctx);
            return self->main_request_handler(req);
        },
        .user_ctx  = this,
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
    };
    httpd_register_uri_handler(server, &catch_all_uri);

    // OPTIONS handler for CORS preflight requests
    httpd_uri_t cors_uri = {
        .uri       = "/*",
        .method    = HTTP_OPTIONS,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            httpd_resp_set_status(req, "204 No Content");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
            return httpd_resp_send(req, NULL, 0);
        },
        .user_ctx  = this,
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
    };
    httpd_register_uri_handler(server, &cors_uri); // fucking cors
    
    // Other handlers for usrdta API
    httpd_uri_t usrdta_post_uri = {
        .uri       = "/usrdta/*",
        .method    = HTTP_POST,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            auto self = static_cast<WebInterface*>(req->user_ctx);
            return self->usrdta_request_handler(req);
        },
        .user_ctx  = this,
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
    };
    httpd_register_uri_handler(server, &usrdta_post_uri);
    httpd_uri_t usrdta_put_uri = {
        .uri       = "/usrdta/*",
        .method    = HTTP_PUT,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            auto self = static_cast<WebInterface*>(req->user_ctx);
            return self->usrdta_request_handler(req);
        },
        .user_ctx  = this,
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
    };
    httpd_register_uri_handler(server, &usrdta_put_uri);
    httpd_uri_t usrdta_delete_uri = {
        .uri       = "/usrdta/*",
        .method    = HTTP_DELETE,
        .handler   = [](httpd_req_t *req) -> esp_err_t {
            auto self = static_cast<WebInterface*>(req->user_ctx);
            return self->usrdta_request_handler(req);
        },
        .user_ctx  = this,
        .is_websocket = false,
        .handle_ws_control_frames = false,
        .supported_subprotocol = nullptr,
    };
    httpd_register_uri_handler(server, &usrdta_delete_uri);

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
    strncpy(my_ip, wifi_manager->getIPAddr(), sizeof(my_ip) - 1);
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

    // if request starts with /usrdta, use the usrdta handler
    if (strncmp(req->uri, "/usrdta", 7) == 0)
    {
        return usrdta_request_handler(req);
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
    Status wifi_err = Robot::GetInstance().getNetworkManager().getWiFiManager().connectToAP(ssid, password);
    if (wifi_err != Status::Ok)
    {
        return httpd_resp_send(req, "Failed to connect to WiFi", HTTPD_RESP_USE_STRLEN);
    }

    return httpd_resp_send(req, "Connecting to WiFi...", HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebInterface::usrdta_request_handler(httpd_req_t *req)
{
    char path[64];
    snprintf(path, sizeof(path), "%s%s", LittleFS::USERDATA_ROOT_FOLDER, req->uri + 7); // +7 to skip "/usrdta"

    if (strcmp(path, LittleFS::USERDATA_ROOT_FOLDER) == 0)
    {
        // Consider as root folder
        snprintf(path, sizeof(path), "%s/", LittleFS::USERDATA_ROOT_FOLDER);
    }

    // Remove any query parameters from the path
    char* query_pos = strchr(path, '?');
    if (query_pos != nullptr)
    {
        *query_pos = '\0'; // Terminate the string at the query position
    }

    // Sanitize the path to prevent invalid characters
    sanitize_filepath(path);

    // little security check to prevent directory traversal attacks
    if (strstr(path, "..") != nullptr)
    {
        return httpd_resp_send(req, "Bad boy, path traversal isn't kind >:(", HTTPD_RESP_USE_STRLEN);
    }

    bool isFolder = path[strlen(path) - 1] == '/';

    // CORS AAAAAHHHHHH
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

    switch (req->method)
    {
    case HTTP_GET:
    {
        if (isFolder)
        {
            // If it's a directory, list it
            httpd_resp_set_type(req, "application/json");
            httpd_resp_sendstr_chunk(req, "[");

            bool firstItem = true;
            Status status = LittleFS::ListDir(path, [&](const char* name, bool isDir) {
                if (!firstItem) httpd_resp_sendstr_chunk(req, ",");
                firstItem = false;

                char jsonChunk[128];
                snprintf(jsonChunk, sizeof(jsonChunk), "{\"name\":\"%s\",\"type\":\"%s\"}", name, isDir ? "directory" : "file");
                
                httpd_resp_sendstr_chunk(req, jsonChunk);
                return true;
            });

            httpd_resp_sendstr_chunk(req, "]");
            httpd_resp_sendstr_chunk(req, NULL); // Indicate end of response
            return status == Status::Ok ? ESP_OK : HTTPD_500_INTERNAL_SERVER_ERROR;
        }
        else
        {
            // If it's a file, serve it
            struct stat st;
            if (stat(path, &st) != 0 || S_ISDIR(st.st_mode))
            {
                return httpd_resp_send_404(req);
            }

            const char* mime_type = get_mime_type(path);
            return send_file_chunked(req, path, mime_type, false);
        }
        break;
    }
    case HTTP_POST:
    {
        if (isFolder)
        {
            // get folder name
            int name_len = req->content_len;
            if (name_len <= 0 || name_len >= 64)
            {
                httpd_resp_set_status(req, "400 Bad Request");
                return httpd_resp_send(req, "Invalid folder name", HTTPD_RESP_USE_STRLEN);
            }
            char folder_name[64];
            if (httpd_req_recv(req, folder_name, name_len) <= 0)
            {
                httpd_resp_set_status(req, "400 Bad Request");
                return httpd_resp_send(req, "Failed to read folder name", HTTPD_RESP_USE_STRLEN);
            }
            sanitize_filepath(folder_name);

            // create the folder
            char new_folder_path[128];
            snprintf(new_folder_path, sizeof(new_folder_path), "%s/%s", path, folder_name);
            if (mkdir(new_folder_path, 0755) != 0)
            {
                httpd_resp_set_status(req, "500 Internal Server Error");
                LOG_ERROR(TAG, "Failed to create folder: %s, error: %d", new_folder_path, errno);
                return httpd_resp_send(req, "Failed to create folder", HTTPD_RESP_USE_STRLEN);
            }

            return httpd_resp_send(req, "Folder created", HTTPD_RESP_USE_STRLEN);
        }

        // open file in write mode
        FILE* f = fopen(path, "w");
        if (!f)
        {
            httpd_resp_set_status(req, "500 Internal Server Error");
            LOG_ERROR(TAG, "Failed to create file: %s, error: %d", path, errno);
            return httpd_resp_send(req, "Failed to create file", HTTPD_RESP_USE_STRLEN);
        }

        int remaining = req->content_len;
        char buf[1024]; // Receive buffer (1kB chunks)
        
        while (remaining > 0)
        {
            // read file chunk
            int received = httpd_req_recv(req, buf, std::min(remaining, (int)sizeof(buf)));
            
            if (received <= 0)
            {
                // timeout or error
                fclose(f);
                remove(path); // Remove the incomplete file
                httpd_resp_set_status(req, "500 Internal Server Error");
                return httpd_resp_send(req, "Upload aborted", HTTPD_RESP_USE_STRLEN);
            }

            // Write chunk to the disk
            if (fwrite(buf, 1, received, f) != received)
            {
                fclose(f);
                remove(path);
                httpd_resp_set_status(req, "500 Internal Server Error");
                LOG_ERROR(TAG, "Disk write error while uploading file: %s, error: %d", path, errno);
                return httpd_resp_send(req, "Disk write error", HTTPD_RESP_USE_STRLEN);
            }
            
            remaining -= received;
        }

        fclose(f);
        httpd_resp_set_status(req, "201 Created"); // noice
        return httpd_resp_send(req, "File uploaded successfully", HTTPD_RESP_USE_STRLEN);
        break;
    }
    case HTTP_DELETE:
    {
        struct stat st;
        if (stat(path, &st) != 0) return httpd_resp_send_404(req);

        if (int ret = remove(path); ret == 0)
        {
            httpd_resp_set_status(req, "200 OK");
            return httpd_resp_send(req, "File deleted", HTTPD_RESP_USE_STRLEN);
        }
        else
        {
            if (errno == EISDIR)
            {
                if (Status err = LittleFS::DeleteDir(path); err == Status::Ok)
                {
                    httpd_resp_set_status(req, "200 OK");
                    return httpd_resp_send(req, "Folder deleted", HTTPD_RESP_USE_STRLEN);
                }
                else
                {
                    httpd_resp_set_status(req, "500 Internal Server Error");
                    LOG_ERROR(TAG, "Failed to delete folder: %s, error: %d", path, errno);
                    return httpd_resp_send(req, "Failed to delete folder", HTTPD_RESP_USE_STRLEN);
                }
            }
            else
            {
                httpd_resp_set_status(req, "500 Internal Server Error");
                LOG_ERROR(TAG, "Failed to delete file: %s, error: %d", path, errno);
                return httpd_resp_send(req, "Failed to delete file", HTTPD_RESP_USE_STRLEN);
            }

        }
        break;
    }
    case HTTP_PUT:
    {
        // URL Query (ex: ?rename=new_name.txt)
        char new_name[64] = {0};
        char query[128];
        
        if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
        {
            if (httpd_query_key_value(query, "rename", new_name, sizeof(new_name)) == ESP_OK)
            {
                char new_path[128];
                snprintf(new_path, sizeof(new_path), "%s/%s", LittleFS::USERDATA_ROOT_FOLDER, new_name);

                // Rename the file
                if (int ret = rename(path, new_path); ret == 0)
                {
                    httpd_resp_set_status(req, "200 OK");
                    return httpd_resp_send(req, "File renamed", HTTPD_RESP_USE_STRLEN);
                }
                else
                {
                    httpd_resp_set_status(req, "500 Internal Server Error");
                    LOG_ERROR(TAG, "Failed to rename file from %s to %s. Error code: %d", path, new_path, errno);
                    return httpd_resp_send(req, "Failed to rename", HTTPD_RESP_USE_STRLEN);
                }
            }
        }
        
        // If query parameter is missing or invalid
        httpd_resp_set_status(req, "400 Bad Request");
        return httpd_resp_send(req, "Missing 'rename' query parameter", HTTPD_RESP_USE_STRLEN);
        break;
    }
    default:
        break;
    }

    return ESP_OK;
}