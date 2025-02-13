#include "storage.hpp"
#include <esp_err.h>
#include <nvs_flash.h>
#include <nvs.h>

nvs_handle_t handle;

esp_err_t storage_init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));

    return ESP_OK;
}

esp_err_t storage_set_str(const char* key, char* value)
{
    esp_err_t err = nvs_set_str(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_str(const char* key, char* value, size_t* length)
{
    return nvs_get_str(handle, key, value, length);
}

esp_err_t storage_set_u8(const char* key, uint8_t value)
{
    esp_err_t err = nvs_set_u8(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_u8(const char* key, uint8_t* value)
{
    return nvs_get_u8(handle, key, value);
}

esp_err_t storage_set_u16(const char* key, uint16_t value)
{
    esp_err_t err = nvs_set_u16(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_u16(const char* key, uint16_t* value)
{
    return nvs_get_u16(handle, key, value);
}

esp_err_t storage_set_u32(const char* key, uint32_t value)
{
    esp_err_t err = nvs_set_u32(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_u32(const char* key, uint32_t* value)
{
    return nvs_get_u32(handle, key, value);
}

esp_err_t storage_set_u64(const char* key, uint64_t value)
{
    esp_err_t err = nvs_set_u64(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_u64(const char* key, uint64_t* value)
{
    return nvs_get_u64(handle, key, value);
}

esp_err_t storage_set_i8(const char* key, int8_t value)
{
    esp_err_t err = nvs_set_i8(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_i8(const char* key, int8_t* value)
{
    return nvs_get_i8(handle, key, value);
}

esp_err_t storage_set_i16(const char* key, int16_t value)
{
    esp_err_t err = nvs_set_i16(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_i16(const char* key, int16_t* value)
{
    return nvs_get_i16(handle, key, value);
}

esp_err_t storage_set_i32(const char* key, int32_t value)
{
    esp_err_t err = nvs_set_i32(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_i32(const char* key, int32_t* value)
{
    return nvs_get_i32(handle, key, value);
}

esp_err_t storage_set_i64(const char* key, int64_t value)
{
    esp_err_t err = nvs_set_i64(handle, key, value);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_i64(const char* key, int64_t* value)
{
    return nvs_get_i64(handle, key, value);
}

esp_err_t storage_set_blob(const char* key, void* value, size_t length)
{
    esp_err_t err = nvs_set_blob(handle, key, value, length);
    if (err != ESP_OK) return err;
    return nvs_commit(handle);
}

esp_err_t storage_get_blob(const char* key, void* value, size_t* length)
{
    return nvs_get_blob(handle, key, value, length);
}

esp_err_t storage_erase(const char* key)
{
    return nvs_erase_key(handle, key);
}

esp_err_t storage_erase_all()
{
    return nvs_erase_all(handle);
}

