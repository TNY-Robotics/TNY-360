#include "common/LittleFS.hpp"
#include "common/Log.hpp"
#include "esp_littlefs.h"
#include <cstring>

namespace LittleFS
{
    bool initialized = false;
    constexpr const char* ROOT_FOLDER = "/storage";
    constexpr const char* TAG = "LittleFS";

    Status Init()
    {
        LOG_SCOPE(TAG, "LittleFS::Init");
        
        if (initialized) return Status::Ok;

        // Mount LittleFS
        esp_vfs_littlefs_conf_t conf = {
            .base_path = ROOT_FOLDER,
            .partition_label = "storage",
            .partition = nullptr,
            .blockdev = nullptr,
            .format_if_mount_failed = true,
            .read_only = false,
            .dont_mount = false,
            .grow_on_mount = false
        };
        esp_err_t ret = esp_vfs_littlefs_register(&conf);

        if (ret != ESP_OK) {
            LOG_ERROR(TAG, "Failed to mount LittleFS");
            // ErrorHandle(ErrorStruct::FileSystemInitFailed);
            return Status::Unknown;
        }

        initialized = true;
        return Status::Ok;
    }

    Status LoadFileContent(const char* path, char** out_buffer, size_t* out_size)
    {
        if (!initialized)
        {
            Init();
        }

        if (!path)
        {
            return Status::InvalidParameters;
        }

        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", ROOT_FOLDER, path);

        // Load index.html content into memory
        FILE* f = fopen(full_path, "r");
        if (f == nullptr)
        {
            return Status::NotFound;
        }

        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (!out_buffer)
        {
            if (!out_size)
            {
                return Status::InvalidParameters;
            }
            *out_size = file_size;
            fclose(f);
            return Status::Ok;
        }

        *out_buffer = new char[file_size + 1];
        fread(*out_buffer, 1, file_size, f);
        (*out_buffer)[file_size] = '\0'; // Null-terminate the string
        
        if (out_size)
        {
            *out_size = file_size;
        }

        fclose(f);

        return Status::Ok;
    }
}