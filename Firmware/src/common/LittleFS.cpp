#include "common/LittleFS.hpp"
#include "common/Log.hpp"
#include "esp_littlefs.h"
#include <cstring>

namespace LittleFS
{
    bool initialized = false;
    constexpr const char* ROOT_FOLDER = "/storage";
    constexpr const char* TAG = "LittleFS";

    Error Init()
    {
        if (initialized) {
            return Error::None;
        }

        // Mount LittleFS
        esp_vfs_littlefs_conf_t conf = {
            .base_path = ROOT_FOLDER,
            .partition_label = "storage",
            .format_if_mount_failed = true,
            .dont_mount = false,
        };
        esp_err_t ret = esp_vfs_littlefs_register(&conf);

        if (ret != ESP_OK) {
            Log::Add(Log::Level::Error, TAG, "Failed to mount LittleFS");
            return Error::Unknown;
        }

        initialized = true;
        return Error::None;
    }

    Error LoadFileContent(const char* path, char** out_buffer, size_t* out_size)
    {
        if (!initialized)
        {
            Init();
        }

        if (!path)
        {
            return Error::InvalidParameters;
        }

        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", ROOT_FOLDER, path);

        // Load index.html content into memory
        FILE* f = fopen(full_path, "r");
        if (f == nullptr)
        {
            return Error::NotFound;
        }

        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (!out_buffer)
        {
            if (!out_size)
            {
                return Error::InvalidParameters;
            }
            *out_size = file_size;
            fclose(f);
            return Error::None;
        }

        *out_buffer = new char[file_size + 1];
        fread(*out_buffer, 1, file_size, f);
        (*out_buffer)[file_size] = '\0'; // Null-terminate the string
        
        if (out_size)
        {
            *out_size = file_size;
        }

        fclose(f);

        return Error::None;
    }
}