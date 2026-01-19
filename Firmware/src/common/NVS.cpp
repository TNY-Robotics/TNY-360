#include "common/NVS.hpp"
#include "common/Log.hpp"
#include "nvs_flash.h"

namespace NVS
{
    bool initialized = false;

    constexpr const char* TAG = "NVS";

    class HandleImpl : public Handle
    {
    public:
        HandleImpl(nvs_handle_t handle) : m_handle(handle) {}

        Error get(const char* key, void* out_value, size_t* length) override
        {
            esp_err_t ret = nvs_get_blob(m_handle, key, out_value, length);
            if (ret == ESP_OK) {
                return Error::None;
            } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
                return Error::NotFound;
            } else {
                return Error::Unknown;
            }
        }

        Error set(const char* key, const void* value, size_t length) override
        {
            esp_err_t ret = nvs_set_blob(m_handle, key, value, length);
            if (ret == ESP_OK) {
                ret = nvs_commit(m_handle);
                if (ret == ESP_OK) {
                    return Error::None;
                } else {
                    return Error::Unknown;
                }
            } else {
                return Error::Unknown;
            }
        }

        Error erase(const char* key) override
        {
            esp_err_t ret = nvs_erase_key(m_handle, key);
            if (ret == ESP_OK) {
                ret = nvs_commit(m_handle);
                if (ret == ESP_OK) {
                    return Error::None;
                } else {
                    return Error::Unknown;
                }
            } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
                return Error::NotFound;
            } else {
                return Error::Unknown;
            }
        }
        
        nvs_handle_t m_handle;
    };

    Error Init()
    {
        if (initialized) {
            return Error::None;
        }

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            Log::Add(Log::Level::Warning, TAG, "NVS flash init failed, erasing and retrying...");
            if (nvs_flash_erase() != ESP_OK) {
                Log::Add(Log::Level::Error, TAG, "Failed to erase NVS flash");
                return Error::SoftwareFailure;
            }
            ret = nvs_flash_init();
        }
        
        if (ret != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to initialize NVS flash");
            return Error::SoftwareFailure;
        }

        initialized = true;

        return Error::None;
    }

    Error Open(const char* namespace_name, Handle** out_handle)
    {
        if (!initialized)
        {
            Error err = Init();
            if (err != Error::None) {
                return err;
            }
        }

        if (!namespace_name || !out_handle)
        {
            return Error::InvalidParameters;
        }

        nvs_handle_t nvs_handle;
        esp_err_t ret = nvs_open(namespace_name, NVS_READWRITE, &nvs_handle);
        if (ret != ESP_OK)
        {
            if (ret == ESP_ERR_NVS_NOT_FOUND) {
                Log::Add(Log::Level::Error, TAG, "NVS namespace not found: %s", namespace_name);
                return Error::NotFound;
            } else {
                Log::Add(Log::Level::Error, TAG, "Failed to open NVS namespace: %s - Error: 0x%x", namespace_name, ret);
                return Error::Unknown;
            }
        }

        *out_handle = reinterpret_cast<Handle*>(new HandleImpl(nvs_handle));
        return Error::None;
    }

    void Close(Handle* handle)
    {
        if (handle)
        {
            HandleImpl* impl = reinterpret_cast<HandleImpl*>(handle);
            nvs_close(impl->m_handle);
            delete impl;
        }
    }
}