#include "network/WebInterface.hpp"
#include "common/Log.hpp"
#include "common/LittleFS.hpp"
#include "locomotion/MotorController.hpp"
#include "Robot.hpp"

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
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_open_sockets = 6;
    config.lru_purge_enable = true;
    config.recv_wait_timeout = 5;
    config.send_wait_timeout = 5;

    if (httpd_start(&server, &config) != ESP_OK)
    {
        server = nullptr;
        Log::Add(Log::Level::Error, TAG, "Failed to start web server");
        return Error::Unknown;
    }
    running = true;

    if (Error err = LittleFS::Init(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to initialize LittleFS");
        return err;
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
        .user_ctx  = this
    };
    httpd_register_uri_handler(server, &catch_all_uri);

    // Note: The 404 handler is less useful here because "/*" catches everything, 
    // except if the method is not GET (e.g., POST/PUT)
    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, [](httpd_req_t *req, httpd_err_code_t error) -> esp_err_t {
       // Your Captive Portal logic or iOS redirection
       httpd_resp_set_status(req, "302 Temporary Redirect");
       httpd_resp_set_hdr(req, "Location", "/");
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

esp_err_t WebInterface::send_file_chunked(httpd_req_t *req, const char *filepath, const char *mime_type, bool is_gzip) {
    FILE *fd = fopen(filepath, "r");
    if (!fd) {
        Log::Add(Log::Level::Error, TAG, "Failed to read file: %s", filepath);
        return HTTPD_500_INTERNAL_SERVER_ERROR;
    }

    httpd_resp_set_type(req, mime_type);
    if (is_gzip) {
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    }

    char *chunk = (char*)malloc(4096); // 4KB buffer
    if (!chunk) {
        fclose(fd);
        return HTTPD_500_INTERNAL_SERVER_ERROR;
    }

    size_t chunksize;
    do {
        chunksize = fread(chunk, 1, 4096, fd);
        if (chunksize > 0) {
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
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

esp_err_t WebInterface::main_request_handler(httpd_req_t *req) {
    
    // 1. Clean the URI (remove query params ?x=1...)
    char filepath[600];
    // req->uri starts with "/"
    snprintf(filepath, sizeof(filepath), "%s%s", MOUNT_POINT, req->uri);
    
    char *query = strchr(filepath, '?');
    if (query) *query = '\0';

    // 2. Handle root "/"
    if (filepath[strlen(filepath) - 1] == '/') {
        strcat(filepath, "index.html");
    }

    Log::Add(Log::Level::Debug, TAG, "Requesting: %s", filepath);

    // 3. FILE STRATEGY:
    // A. Does a compressed .gz version exist? (TOP PRIORITY)
    char filepath_gz[610];
    snprintf(filepath_gz, sizeof(filepath_gz), "%s.gz", filepath);
    
    struct stat st;
    if (stat(filepath_gz, &st) == 0) {
        // The .gz exists! We send it.
        // Note: we pass the original path to guess the mime type (e.g., style.css)
        return send_file_chunked(req, filepath_gz, get_mime_type(filepath), true);
    }

    // B. Does the file exist as is? (e.g., uncompressed images, js, css)
    if (stat(filepath, &st) == 0) {
        return send_file_chunked(req, filepath, get_mime_type(filepath), false);
    }

    // 4. SPA FALLBACK (Client-Side Routing)
    // If the file doesn't exist, and the URL doesn't look like an asset (no extension or not in _nuxt)
    // We return index.html.gz so Nuxt can handle the route.

    // FIXME : This is not working. stat doesnt return if the file exists or not.
    //         We should check the return value of send_file_chunked instead and fallback to index file.
    
    // If the URI contains "/_nuxt/" or a common extension, it's a real 404
    if (strstr(filepath, "/_nuxt/") || strstr(filepath, "/assets/")) {
        return httpd_resp_send_404(req);
    }

    // Otherwise, it's a virtual route (e.g., /calibration), we serve the index
    Log::Add(Log::Level::Info, TAG, "SPA Fallback: Redirecting to index.html.gz");
    char index_path[64];
    snprintf(index_path, sizeof(index_path), "%s/index.html.gz", MOUNT_POINT);
    
    if (stat(index_path, &st) == 0) {
        return send_file_chunked(req, index_path, "text/html", true);
    }

    return httpd_resp_send_404(req);
}