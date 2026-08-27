#include "common/LittleFS.hpp"
#include "common/Log.hpp"
#include "esp_littlefs.h"
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <stdio.h>
#include <unistd.h>

namespace LittleFS
{
    bool initialized = false;
    constexpr const char* TAG = "LittleFS";

    Status Init()
    {
        LOG_SCOPE(TAG, "LittleFS::Init");
        
        if (initialized) return Status::Ok;

        // Mount filesystem partition
        esp_vfs_littlefs_conf_t conf = {
            .base_path = FILESYSTEM_ROOT_FOLDER,
            .partition_label = "storage",
            .partition = nullptr,
            .blockdev = nullptr,
            .format_if_mount_failed = true,
            .read_only = false,
            .dont_mount = false,
            .grow_on_mount = false
        };
        if (esp_err_t ret = esp_vfs_littlefs_register(&conf); ret != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to mount filesystem partition. Error code: %d", ret);
            // ErrorHandle(ErrorStruct::FileSystemInitFailed);
            return Status::Unknown;
        }

        // Mount userdata partition
        esp_vfs_littlefs_conf_t userdata_conf = {
            .base_path = USERDATA_ROOT_FOLDER,
            .partition_label = "userdata",
            .partition = nullptr,
            .blockdev = nullptr,
            .format_if_mount_failed = true,
            .read_only = false,
            .dont_mount = false,
            .grow_on_mount = false
        };
        if (esp_err_t ret = esp_vfs_littlefs_register(&userdata_conf); ret != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to mount userdata partition. Error code: %d", ret);
            // ErrorHandle(ErrorStruct::UserDataInitFailed);
            return Status::Unknown;
        }

        initialized = true;
        return Status::Ok;
    }

    Status Deinit()
    {
        LOG_SCOPE(TAG, "LittleFS::Deinit");
        
        if (!initialized) return Status::Ok;

        // Unmount filesystem partition
        if (esp_err_t ret = esp_vfs_littlefs_unregister(FILESYSTEM_ROOT_FOLDER); ret != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to unmount filesystem partition. Error code: %d", ret);
            return Status::Unknown;
        }

        // Unmount userdata partition
        if (esp_err_t ret = esp_vfs_littlefs_unregister(USERDATA_ROOT_FOLDER); ret != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to unmount userdata partition. Error code: %d", ret);
            return Status::Unknown;
        }

        initialized = false;
        return Status::Ok;
    }

    Status GetFileSize(const char* path, size_t& size)
    {
        LOG_SCOPE(TAG, "LittleFS::GetFileSize");

        if (!initialized) return Status::InvalidState;
        if (path == nullptr) return Status::InvalidParameters;

        struct stat st;
        if (stat(path, &st) != 0)
        {
            LOG_ERROR(TAG, "Failed to get file size for path: %s", path);
            return Status::NotFound;
        }

        size = st.st_size;
        return Status::Ok;
    }

    Status ReadFile(const char* path, char* buffer, size_t bufferSize, size_t& bytesRead)
    {
        LOG_SCOPE(TAG, "LittleFS::ReadFile");

        if (!initialized) return Status::InvalidState;
        if (path == nullptr || buffer == nullptr) return Status::InvalidParameters;

        FILE* file = fopen(path, "rb");
        if (file == nullptr)
        {
            LOG_ERROR(TAG, "Failed to open file for reading: %s", path);
            return Status::NotFound;
        }

        bytesRead = fread(buffer, 1, bufferSize, file);
        fclose(file);

        if (bytesRead == 0)
        {
            LOG_ERROR(TAG, "Failed to read file: %s", path);
            return Status::Failure;
        }

        return Status::Ok;
    }

    Status WriteFile(const char* path, char* data, size_t dataSize)
    {
        LOG_SCOPE(TAG, "LittleFS::WriteFile");

        if (!initialized) return Status::InvalidState;
        if (path == nullptr || data == nullptr) return Status::InvalidParameters;

        FILE* file = fopen(path, "wb");
        if (file == nullptr)
        {
            LOG_ERROR(TAG, "Failed to open file for writing: %s", path);
            return Status::NotFound;
        }

        size_t bytesWritten = fwrite(data, 1, dataSize, file);
        fclose(file);

        if (bytesWritten != dataSize)
        {
            LOG_ERROR(TAG, "Failed to write all data to file: %s", path);
            return Status::Failure;
        }

        return Status::Ok;
    }

    Status ListDir(const char* path, std::function<bool(const char*, bool)> callback)
    {
        DIR* dir = opendir(path);
        if (!dir) return Status::NotFound;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            bool isDir = (entry->d_type == DT_DIR);
            if (!callback(entry->d_name, isDir)) break;
        }
        
        closedir(dir);
        return Status::Ok;
    }

    Status DeleteDir(const char* path)
    {
        DIR* dir = opendir(path);
        if (!dir) return Status::NotFound;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string fullPath = std::string(path) + "/" + entry->d_name;
            if (entry->d_type == DT_DIR)
            {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                {
                    if (Status err = DeleteDir(fullPath.c_str()); err != Status::Ok)
                    {
                        closedir(dir);
                        return err;
                    }
                }
            }
            else
            {
                if (remove(fullPath.c_str()) != 0)
                {
                    LOG_ERROR(TAG, "Failed to delete file: %s", fullPath.c_str());
                    closedir(dir);
                    return Status::Failure;
                }
            }
        }

        closedir(dir);
        if (rmdir(path) != 0)
        {
            LOG_ERROR(TAG, "Failed to delete directory: %s", path);
            return Status::Failure;
        }

        return Status::Ok;
    }
}