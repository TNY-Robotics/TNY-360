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

        Status get(const char* key, void* out_value, size_t* length) override
        {
            esp_err_t ret = nvs_get_blob(m_handle, key, out_value, length);
            if (ret == ESP_OK) {
                return Status::Ok;
            } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
                return Status::NotFound;
            } else {
                return Status::Unknown;
            }
        }

        Status set(const char* key, const void* value, size_t length) override
        {
            esp_err_t ret = nvs_set_blob(m_handle, key, value, length);
            if (ret == ESP_OK) {
                ret = nvs_commit(m_handle);
                if (ret == ESP_OK) {
                    return Status::Ok;
                } else {
                    return Status::Unknown;
                }
            } else {
                return Status::Unknown;
            }
        }

        Status erase(const char* key) override
        {
            esp_err_t ret = nvs_erase_key(m_handle, key);
            if (ret == ESP_OK) {
                ret = nvs_commit(m_handle);
                if (ret == ESP_OK) {
                    return Status::Ok;
                } else {
                    return Status::Unknown;
                }
            } else if (ret == ESP_ERR_NVS_NOT_FOUND) {
                return Status::NotFound;
            } else {
                return Status::Unknown;
            }
        }
        
        nvs_handle_t m_handle;
    };

    Status Init()
    {
        LOG_SCOPE(TAG, "NVS::Init");

        if (initialized) return Status::Ok;

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            LOG_WARNING(TAG, "NVS flash init failed, erasing and retrying...");
            if (nvs_flash_erase() != ESP_OK) {
                LOG_ERROR(TAG, "Failed to erase NVS flash");
                // ErrorHandle(ErrorStruct::NVSInitFailed);
                return Status::Failure;
            }
            ret = nvs_flash_init();
        }
        
        if (ret != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to initialize NVS flash");
            // ErrorHandle(ErrorStruct::NVSInitFailed);
            return Status::Failure;
        }

        initialized = true;
        return Status::Ok;
    }

    Status Open(const char* namespace_name, Handle** out_handle)
    {
        LOG_SCOPE(TAG, "NVS::Open");
        if (!initialized)
        {
            Status err = Init();
            if (err != Status::Ok) {
                return err;
            }
        }

        if (!namespace_name || !out_handle)
        {
            return Status::InvalidParameters;
        }

        nvs_handle_t nvs_handle;
        esp_err_t ret = nvs_open(namespace_name, NVS_READWRITE, &nvs_handle);
        if (ret != ESP_OK)
        {
            if (ret == ESP_ERR_NVS_NOT_FOUND) {
                LOG_ERROR(TAG, "NVS namespace not found: %s", namespace_name);
                return Status::NotFound;
            } else {
                LOG_ERROR(TAG, "Failed to open NVS namespace: %s - Error: 0x%x", namespace_name, ret);
                return Status::Unknown;
            }
        }

        *out_handle = reinterpret_cast<Handle*>(new HandleImpl(nvs_handle));
        return Status::Ok;
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