#pragma once
#include <esp_err.h>

/** initialize nvs storage */
esp_err_t storage_init();

/** set str type nvs data at given key */
esp_err_t storage_set_str(const char* key, char* value);

/** get str type nvs data from given key */
esp_err_t storage_get_str(const char* key, char* value, size_t* length);

/** set _u8 type nvs data at given key */
esp_err_t storage_set_u8(const char* key, uint8_t* value);

/** get _u8 type nvs data from given key */
esp_err_t storage_get_u8(const char* key, uint8_t value);

/** set u16 type nvs data at given key */
esp_err_t storage_set_u16(const char* key, uint16_t* value);

/** get u16 nvs data from given key */
esp_err_t storage_get_u16(const char* key, uint16_t value);

/** set u32 type nvs data at given key */
esp_err_t storage_set_u32(const char* key, uint32_t* value);

/** get u32 type nvs data from given key */
esp_err_t storage_get_u32(const char* key, uint32_t value);

/** set u64 type nvs data at given key */
esp_err_t storage_set_u64(const char* key, uint64_t* value);

/** get u64 type nvs data from given key */
esp_err_t storage_get_u64(const char* key, uint64_t value);

/** set _i8 type nvs data at given key */
esp_err_t storage_set_i8(const char* key, int8_t* value);

/** get _i8 type nvs data from given key */
esp_err_t storage_get_i8(const char* key, int8_t value);

/** set i16 type nvs data at given key */
esp_err_t storage_set_i16(const char* key, int16_t* value);

/** get i16 type nvs data from given key */
esp_err_t storage_get_i16(const char* key, int16_t value);

/** set i32 type nvs data at given key */
esp_err_t storage_set_i32(const char* key, int32_t* value);

/** get i32 type nvs data from given key */
esp_err_t storage_get_i32(const char* key, int32_t value);

/** set i64 type nvs data at given key */
esp_err_t storage_set_i64(const char* key, int64_t* value);

/** get i64 type nvs data from given key */
esp_err_t storage_get_i64(const char* key, int64_t value);

/** set blob type nvs data at given key */
esp_err_t storage_set_blob(const char* key, void* value, size_t length);

/** get blob type nvs data from given key */
esp_err_t storage_get_blob(const char* key, void* value, size_t* length);

/** erase nvs data at given key */
esp_err_t storage_erase(const char* key);

/** erase all nvs data */
esp_err_t storage_erase_all();
