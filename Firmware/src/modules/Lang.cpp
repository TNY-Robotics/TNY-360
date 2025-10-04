#include <cstdint>
#include <cstring>
#include <cmath>
#include "modules/Lang.hpp"
#include "modules/Log.hpp"
#include "esp_littlefs.h"

FILE* file_handle = NULL;

constexpr uint8_t CACHE_SIZE = 16;
constexpr uint8_t MAX_TRANS_LEN = 32;

typedef struct CacheEntry
{
    Lang::KEY key;
    char translation[MAX_TRANS_LEN];
} CacheEntry;

CacheEntry cache[CACHE_SIZE] = {};
uint8_t cache_count = 0;
uint8_t cache_index = 0;

uint32_t* offset_table = nullptr;
uint32_t nb_entries = 0;

bool Lang::Init()
{
    Log::Add(Log::Level::Info, "Initializing Language Module");
    
    // Mount LittleFS
    // esp_vfs_littlefs_conf_t conf = {
    //     .base_path = "/storage",
    //     .partition_label = "storage",
    //     .format_if_mount_failed = true,
    //     .dont_mount = false
    // };
    // esp_err_t ret = esp_vfs_littlefs_register(&conf);

    // if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    //     Log::Add(Log::Level::Error, "Failed to mount LittleFS");
    //     return false;
    // }

    // FIXME : littlefs is already mounted in WebServer.cpp, this should be moved to a separated FS module

    // Load lang.bin content into memory
    file_handle = fopen("/storage/lang.bin", "r");
    if (file_handle == nullptr) {
        Log::Add(Log::Level::Error, "Failed to open lang.bin");
        return false;
    }

    // read the version (1 byte)
    uint8_t version = 0;
    fread(&version, 1, 1, file_handle);
    Log::Add(Log::Level::Info, "Language file version: %d", version);

    fread(&nb_entries, 4, 1, file_handle);
    Log::Add(Log::Level::Info, "Number of entries: %d", nb_entries);

    offset_table = new uint32_t[nb_entries];
    fread(offset_table, 4, nb_entries, file_handle);
    Log::Add(Log::Level::Info, "Offset table loaded");

    Log::Add(Log::Level::Info, "Language Module initialized");
    return true;
}

const char* Lang::Get(Lang::KEY key)
{
    // Check cache first
    for (int i = 0; i < cache_count; i++) {
        if (cache[i].key == key) {
            return cache[i].translation;
        }
    }

    if (file_handle == nullptr || offset_table == nullptr || nb_entries == 0) {
        Log::Add(Log::Level::Error, "Language module not initialized");
        return "N/A";
    }

    uint32_t index = static_cast<uint32_t>(key);
    if (index >= nb_entries) {
        Log::Add(Log::Level::Error, "Invalid language key: %d", index);
        return "N/A";
    }

    // Seek to the offset
    fseek(file_handle, offset_table[index], SEEK_SET);
    char buffer[MAX_TRANS_LEN];
    fread(buffer, 1, MAX_TRANS_LEN, file_handle);
    buffer[MAX_TRANS_LEN - 1] = '\0';  // Ensure null-termination

    // Cache the translation
    cache[cache_index].key = key;
    strncpy(cache[cache_index].translation, buffer, MAX_TRANS_LEN);
    cache_index = (cache_index + 1) % CACHE_SIZE;
    cache_count = std::min<uint8_t>(cache_count + 1, CACHE_SIZE);

    return cache[(cache_index + CACHE_SIZE - 1) % CACHE_SIZE].translation;
}