#include "drivers/CameraDriver.hpp"
#include "common/Log.hpp"
#include "common/I2C.hpp"
#include "Robot.hpp"

#include "driver/i2c_master.h"
#include "esp_timer.h"

constexpr const char* TAG = "CameraDriver";

static camera_config_t camera_config = {
    .pin_pwdn  = -1,
    .pin_reset = -1,
    .pin_xclk = GPIO_NUM_8,
    .pin_sccb_sda = -1, // -1 because i2c port should already be initialized
    .pin_sccb_scl = -1, // -1 because i2c port should already be initialized

    .pin_d7 = GPIO_NUM_19,
    .pin_d6 = GPIO_NUM_18,
    .pin_d5 = GPIO_NUM_17,
    .pin_d4 = GPIO_NUM_15,
    .pin_d3 = GPIO_NUM_6,
    .pin_d2 = GPIO_NUM_4,
    .pin_d1 = GPIO_NUM_5,
    .pin_d0 = GPIO_NUM_7,
    .pin_vsync = GPIO_NUM_3,
    .pin_href = GPIO_NUM_20,
    .pin_pclk = GPIO_NUM_16,

    .xclk_freq_hz = 20000000, // 20Mhz
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_VGA,

    .jpeg_quality = 20,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,

    .sccb_i2c_port = I2C_NUM_1, // using secondary i2c bus (the "slow" one)
};

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

esp_err_t jpg_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        LOG_ERROR(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    if(res == ESP_OK){
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    LOG_INFO(TAG, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}

esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = nullptr;
    esp_err_t res = ESP_OK;
    char *part_buf[128];

    // Send multipart header
    res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=" "____boundary____");
    if (res != ESP_OK) return res;

    int64_t last_frame_time = esp_timer_get_time();

    while (true) {
        // Get the image
        fb = esp_camera_fb_get();
        if (!fb) {
            LOG_WARNING(TAG, "Failed to get camera capture");
            res = ESP_FAIL;
            break;
        }

        size_t hlen = snprintf((char *)part_buf, 128, 
            "--%s\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", 
            "____boundary____", fb->len);
        
        // Send image header
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        if (res != ESP_OK) {
            esp_camera_fb_return(fb);
            break; // disconnected
        }

        // Send image data
        res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        
        // Free camera buffer
        esp_camera_fb_return(fb);
        
        if (res != ESP_OK) {
            break;
        }

        // TODO : Fixed framerate with delay
    }

    return res;
}

CameraDriver::CameraDriver()
{
}

CameraDriver::~CameraDriver()
{
}

Error CameraDriver::init()
{
    LOG_SCOPE(TAG, "CameraDriver::init");

    // initialize i2c for camera configuration
    if (Error err = I2C::Init(); err != Error::None)
    {
        return err;
    }

    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        LOG_ERROR(TAG, "Camera init failed with error 0x%x", err);
        return Error::HardwareFailure;
    }

    // flip the image (sensor is upside down)
    sensor = esp_camera_sensor_get();
    if (sensor == nullptr)
    {
        LOG_ERROR(TAG, "esp_camera_sensor_get returned nullptr");
        return Error::Unknown;
    }
    sensor->set_vflip(sensor, true);
    sensor->set_hmirror(sensor, true);

    return Error::None;
}

Error CameraDriver::deinit()
{
    if (esp_err_t err = esp_camera_deinit(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to deinit camera : Error 0x%0x", err);
        return Error::SoftwareFailure;
    }

    return Error::None;
}

Error CameraDriver::start()
{
    // Start the camera stream server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 90; // FIXME : This port shouldn't be hardcoded
    config.max_uri_handlers = 8;
    config.ctrl_port = 90 + 1;
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_open_sockets = 6;
    config.lru_purge_enable = true;
    config.recv_wait_timeout = 5;
    config.send_wait_timeout = 5;

    if (httpd_start(&server, &config) != ESP_OK)
    {
        server = nullptr;
        LOG_ERROR(TAG, "Failed to start web server");
        return Error::SoftwareFailure;
    }

    httpd_uri_t catch_all_uri = {
        .uri       = "/*", // Wildcard
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = this
    };
    if (esp_err_t err = httpd_register_uri_handler(server, &catch_all_uri); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Unable to register URI handler : Error 0x%0x", err);
        return Error::SoftwareFailure;
    }

    return Error::None;
}

Error CameraDriver::stop()
{
    if (server)
    {
        httpd_stop(server);
        server = nullptr;
    }
    return Error::None;
}