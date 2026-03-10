#include "drivers/CameraDriver.hpp"
#include "common/Log.hpp"

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"

constexpr const char* TAG = "CameraDriver";

static camera_config_t camera_config = {
    .pin_pwdn  = -1,
    .pin_reset = -1,
    .pin_xclk = GPIO_NUM_8,
    .pin_sccb_sda = GPIO_NUM_3,
    .pin_sccb_scl = GPIO_NUM_20,

    .pin_d7 = GPIO_NUM_19,
    .pin_d6 = GPIO_NUM_18,
    .pin_d5 = GPIO_NUM_17,
    .pin_d4 = GPIO_NUM_15,
    .pin_d3 = GPIO_NUM_6,
    .pin_d2 = GPIO_NUM_4,
    .pin_d1 = GPIO_NUM_5,
    .pin_d0 = GPIO_NUM_7,
    .pin_vsync = GPIO_NUM_9,
    .pin_href = GPIO_NUM_46,
    .pin_pclk = GPIO_NUM_16,

    .xclk_freq_hz = 10000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_UXGA,//QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY//CAMERA_GRAB_LATEST. Sets when buffers should be filled
};

// static camera_config_t camera_config = {
//     .pin_pwdn  = -1,
//     .pin_reset = -1,
//     .pin_xclk = GPIO_NUM_15,
//     .pin_sccb_sda = GPIO_NUM_4,
//     .pin_sccb_scl = GPIO_NUM_5,

//     .pin_d7 = GPIO_NUM_16,
//     .pin_d6 = GPIO_NUM_17,
//     .pin_d5 = GPIO_NUM_18,
//     .pin_d4 = GPIO_NUM_12,
//     .pin_d3 = GPIO_NUM_10,
//     .pin_d2 = GPIO_NUM_8,
//     .pin_d1 = GPIO_NUM_9,
//     .pin_d0 = GPIO_NUM_11,
//     .pin_vsync = GPIO_NUM_6,
//     .pin_href = GPIO_NUM_7,
//     .pin_pclk = GPIO_NUM_13,

//     .xclk_freq_hz = 10000000,
//     .ledc_timer = LEDC_TIMER_0,
//     .ledc_channel = LEDC_CHANNEL_0,

//     .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
//     .frame_size = FRAMESIZE_UXGA,//QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

//     .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
//     .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
//     .grab_mode = CAMERA_GRAB_WHEN_EMPTY//CAMERA_GRAB_LATEST. Sets when buffers should be filled
// };

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
        Log::Add(Log::Level::Error, TAG, "Camera capture failed");
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
    Log::Add(Log::Level::Info, TAG, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
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
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Log::Add(Log::Level::Error, TAG, "Camera init failed with error 0x%x", err);
        return Error::HardwareFailure;
    }
    return Error::None;
}

Error CameraDriver::deinit()
{
    return Error::None;
}