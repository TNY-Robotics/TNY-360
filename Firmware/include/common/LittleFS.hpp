#pragma once
#include "utils.hpp"
#include <vector>
#include <string>
#include <functional>

namespace LittleFS
{
    constexpr const char* FILESYSTEM_ROOT_FOLDER = "/storage";
    constexpr const char* USERDATA_ROOT_FOLDER = "/userdata";

    struct FileInfo
    {
        std::string name;
        bool isDirectory;
    };

    /**
     * @brief Initialize and mount LittleFS partitions
     * @return Status::Ok if sucessfull
     */
    Status Init();
    /**
     * @brief Deinitialize and unmount LittleFS partitions
     * @return Status::Ok if sucessfull
     */
    Status Deinit();

    /**
     * @brief Get the size of a file in bytes
     * @param path The path to the file
     * @param size Reference to a size_t variable where the file size will be stored
     * @return Status::Ok if successful, or an appropriate error status
     */
    Status GetFileSize(const char* path, size_t& size);

    /**
     * @brief Read the contents of a file into a buffer
     * @param path The path to the file
     * @param buffer The buffer where the file contents will be stored
     * @param bufferSize The size of the buffer
     * @param bytesRead Reference to a size_t variable where the number of bytes read will be stored
     * @return Status::Ok if successful, or an appropriate error status
     */
    Status ReadFile(const char* path, char* buffer, size_t bufferSize, size_t& bytesRead);

    /**
     * @brief Write data to a file
     * @param path The path to the file
     * @param data The data to write
     * @param dataSize The size of the data to write
     * @return Status::Ok if successful, or an appropriate error status
     */
    Status WriteFile(const char* path, char* data, size_t dataSize);

    /**
     * @brief Iterate over all items in a directory
     * @param path The path to the directory
     * @param callback Function called for each item. Takes filename and a boolean (true if it's a directory).
     *                 Return true in the callback to continue listing, false to abort.
     * @return Status::Ok if successful, or an appropriate error status
     */
    Status ListDir(const char* path, std::function<bool(const char* filename, bool isDirectory)> callback);

    /**
     * @brief Delete a directory and its contents
     * @param path The path to the directory
     * @return Status::Ok if successful, or an appropriate error status
     */
    Status DeleteDir(const char* path);
}