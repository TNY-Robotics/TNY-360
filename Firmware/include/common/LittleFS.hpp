#pragma once
#include "utils.hpp"

namespace LittleFS
{
    /**
     * @brief Initialize and mount LittleFS
     * @return Error code
     */
    Error Init();

    /**
     * @brief Load entire file content into given buffer
     * @param path The file path
     * @param out_buffer The buffer to load content into
     * @param out_size The size of the loaded content
     * @return Error code
     * @note If out_buffer is nullptr and out_size is provided, only the size of the file will be returned
     */
    Error LoadFileContent(const char* path, char** out_buffer, size_t* out_size);
}