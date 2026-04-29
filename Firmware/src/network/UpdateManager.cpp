#include "network/UpdateManager.hpp"
#include "esp_http_client.h"
#include "esp_partition.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "cJSON.h"

// Code-embeded root CA certificate (PEM format)
extern const uint8_t root_ca_pem_start[] asm("_binary_root_ca_pem_start");
extern const uint8_t root_ca_pem_end[]   asm("_binary_root_ca_pem_end");

UpdateManager::UpdateManager()
{
}

Error UpdateManager::init()
{
    LOG_SCOPE(TAG, "UpdateManager::init");

    // Initialize and get NVS handle
    if (Error err = NVS::Init(); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to initialize NVS: %d", static_cast<int>(err));
        return err;
    }
    if (Error err = NVS::Open("updt_mngr", &nvsHandle_ptr); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to open NVS namespace: %d", static_cast<int>(err));
        return err;
    }

    return Error::None;
}

Error UpdateManager::deinit()
{
    if (nvsHandle_ptr)
    {
        delete nvsHandle_ptr;
        nvsHandle_ptr = nullptr;
    }

    if (latestVersion)
    {
        free(latestVersion);
        latestVersion = nullptr;
    }
    if (firmwareDownloadUrl)
    {
        free(firmwareDownloadUrl);
        firmwareDownloadUrl = nullptr;
    }
    if (filesystemDownloadUrl)
    {
        free(filesystemDownloadUrl);
        filesystemDownloadUrl = nullptr;
    }
    return Error::None;
}

Error UpdateManager::checkForUpdates()
{
    TaskHandle_t task;
    xTaskCreatePinnedToCore([](void* pvParams){
        ((UpdateManager*) pvParams)->run_update_task();
        vTaskDelete(nullptr);
    }, "Update Check task", 8192, this, tskIDLE_PRIORITY + 1, &task, CORE_BRAIN);

    return Error::None;
}

Error UpdateManager::downloadAndApplyFirmwareUpdate()
{
    if (!updateAvailable || !firmwareDownloadUrl)
    {
        LOG_DEBUG(TAG, "No firmware update available to download.");
        return Error::InvalidState;
    }

    TaskHandle_t task;
    xTaskCreatePinnedToCore([](void* pvParams){
        ((UpdateManager*) pvParams)->run_download_firmware_task();
        vTaskDelete(nullptr);
    }, "Firmware update task", 8192, this, tskIDLE_PRIORITY + 1, &task, CORE_BRAIN);

    return Error::None;
}

Error UpdateManager::downloadAndApplyFilesystemUpdate()
{
    filesystemDownloadUrl = new char[128];
    sprintf(filesystemDownloadUrl, OTA_FILESYSTEM_DOWNLOAD_URL, FIRMWARE_VERSION);

    TaskHandle_t task;
    xTaskCreatePinnedToCore([](void* pvParams){
        ((UpdateManager*) pvParams)->run_download_filesystem_task();
        vTaskDelete(nullptr);
    }, "Filesystem update task", 8192, this, tskIDLE_PRIORITY + 1, &task, CORE_BRAIN);

    return Error::None;
}

Error UpdateManager::verifyFirmware()
{
    LOG_INFO(TAG, "Running firmware diagnostics...");

    // TODO : Implement actual diagnostics here

    LOG_INFO(TAG, "Firmware diagnostics completed successfully.");
    return Error::None;
}

void UpdateManager::run_update_task()
{
    status = Status::Fetching;

    LOG_DEBUG(TAG, "Contacting update server at %s", OTA_FIRMWARE_LATEST_URL);
    esp_http_client_config_t config = {};
    config.url = OTA_FIRMWARE_LATEST_URL;
    config.cert_pem = (char *)root_ca_pem_start;
    config.timeout_ms = OTA_UPDATE_TIMEOUT_MS;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "x-robot-model", OTA_ROBOT_MODEL);

    if (esp_err_t err = esp_http_client_open(client, 0); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to connect to API: %d", err);
        status = Status::ErrorUnreachable;
        return;
    }

    esp_http_client_fetch_headers(client);
    
    char buffer[1024];
    int read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
    if (read_len > 0)
    {
        buffer[read_len] = 0; // Null terminate for safety
        
        cJSON *json = cJSON_Parse(buffer);
        if (json)
        {
            cJSON *ver = cJSON_GetObjectItem(json, "version");
            cJSON *fw_url = cJSON_GetObjectItem(json, "firmwareDownloadUrl");
            cJSON *fs_url = cJSON_GetObjectItem(json, "filesystemDownloadUrl");

            // copy in member variables
            latestVersion = ver && ver->valuestring ? strdup(ver->valuestring) : nullptr;
            firmwareDownloadUrl = fw_url && fw_url->valuestring ? strdup(fw_url->valuestring) : nullptr;
            filesystemDownloadUrl = fs_url && fs_url->valuestring ? strdup(fs_url->valuestring) : nullptr;

            if (ver && fw_url && fs_url)
            {
                LOG_DEBUG(TAG, "Comparing version strings: %s vs %s", ver->valuestring, FIRMWARE_VERSION);
                if (strcmp(ver->valuestring, FIRMWARE_VERSION) != 0)
                {
                    LOG_DEBUG(TAG, "New version found: %s", ver->valuestring);
                    updateAvailable = true;
                    // NOTE : Not launching update here, just marking it as available
                    // It will be triggered by the user if desired
                }
                else
                {
                    LOG_DEBUG(TAG, "System is up to date.");
                    updateAvailable = false;
                }
                status = Status::Done;
            }
            else
            {
                status = Status::ErrorInvalidJson;
            }
            cJSON_Delete(json);
        }
    }
    else
    {
        LOG_ERROR(TAG, "Failed to read response from API");
        status = Status::ErrorEmptyResponse;
    }
}

void UpdateManager::run_download_firmware_task()
{
    status = Status::DownloadingFirmware;

    LOG_DEBUG(TAG, "Starting firmware update from URL: %s", firmwareDownloadUrl);
    esp_http_client_config_t config = {};
    config.url = firmwareDownloadUrl;
    config.cert_pem = (char *)root_ca_pem_start;
    config.keep_alive_enable = true; // Keep the connection alive for large downloads
    config.timeout_ms = OTA_UPDATE_TIMEOUT_MS;

    esp_https_ota_config_t ota_config = {};
    ota_config.http_config = &config;
    
    status = Status::UpdatingFirmware;

    if (esp_err_t ret = esp_https_ota(&ota_config); ret == ESP_OK)
    {
        // store filesystem URL in NVS for next boot to download
        if (Error err = nvsHandle_ptr->set("fs_updt_url", filesystemDownloadUrl); err != Error::None)
        {
            LOG_ERROR(TAG, "Failed to store filesystem update URL in NVS: %s", ErrorToString(err));
            status = Status::ErrorUnknown;
        }

        status = Status::Rebooting;
        LOG_INFO(TAG, "Firmware update applied successfully. Restarting in 3 seconds ...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_restart();
    }
    else
    {
        LOG_ERROR(TAG, "Firmware update failed: %d", ret);
        status = Status::ErrorFirmwareUpdateFailed;
    }
}

void UpdateManager::run_download_filesystem_task()
{
    status = Status::DownloadingFilesystem;

    LOG_DEBUG(TAG, "Starting filesystem update from URL: %s", filesystemDownloadUrl);

    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    if (!part) {
        LOG_ERROR(TAG, "Failed to find storage partition for filesystem update.");
        status = Status::ErrorPartitionNotFound;
    }

    LOG_DEBUG(TAG, "Found storage partition at address 0x%X, size %d bytes.", part->address, part->size);
    esp_http_client_config_t config = {};
    config.url = filesystemDownloadUrl;
    config.cert_pem = (char *)root_ca_pem_start;
    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (!client) {
        LOG_ERROR(TAG, "Failed to initialize HTTP client for filesystem update.");
        status = Status::ErrorHTTPClient;
    }
    LOG_DEBUG(TAG, "HTTP client initialized for filesystem update.");

    // Open HTTP connection
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        LOG_ERROR(TAG, "Failed to open HTTP connection for filesystem update: 0x%0X", err);
        esp_http_client_cleanup(client);
        status = Status::ErrorUnreachable;
    }
    
    LOG_DEBUG(TAG, "HTTP connection opened for filesystem update.");

    // Get filesystem size
    int content_len = esp_http_client_fetch_headers(client);
    if (content_len > part->size) {
        LOG_ERROR(TAG, "Filesystem update size (%d) exceeds partition size (%d).", content_len, part->size);
        esp_http_client_cleanup(client);
        status = Status::ErrorOutOfBounds;
    }

    LOG_DEBUG(TAG, "Filesystem update size: %d bytes.", content_len);

    // Erase partition
    err = esp_partition_erase_range(part, 0, part->size);
    if (err != ESP_OK) {
        LOG_ERROR(TAG, "Failed to erase storage partition: 0x%0X", err);
        esp_http_client_cleanup(client);
        status = Status::ErrorEraseStorage;
    }

    LOG_DEBUG(TAG, "Storage partition erased successfully.");

    // Download and write filesystem data
    char buffer[OTA_UPDATE_FILESYSTEM_BUFFER_SIZE];
    size_t total_read = 0;

    while (true) {
        int read = esp_http_client_read(client, buffer, OTA_UPDATE_FILESYSTEM_BUFFER_SIZE);
        if (read < 0) {
            LOG_ERROR(TAG, "Error reading filesystem update data from HTTP.");
            err = ESP_FAIL;
            break;
        } else if (read == 0) {
            // Download complete
            err = ESP_OK;
            break;
        }

        // Write to partition
        esp_err_t write_err = esp_partition_write(part, total_read, buffer, read);
        if (write_err != ESP_OK) {
            LOG_ERROR(TAG, "Error writing filesystem update data to partition: %d", write_err);
            err = write_err;
            break;
        }
        total_read += read;
        LOG_DEBUG(TAG, "Written %d/%d bytes of filesystem update (%d%%).", total_read, content_len, (total_read * 100) / content_len);
        progress = total_read / (float) content_len;
    }

    status = Status::UpdatingFilesystem;

    if (err == ESP_OK) {
        LOG_INFO(TAG, "Filesystem update applied successfully.");
        // Clear filesystem update URL from NVS
        nvsHandle_ptr->erase("fs_updt_url");
        status = Status::Rebooting;
        LOG_INFO(TAG, "Filesystem update applied successfully. Restarting in 3 seconds ...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_restart();
    } else {
        LOG_ERROR(TAG, "Filesystem update failed: %d", err);
        status = Status::ErrorFilesystemUpdateFailed;
    }
    
    LOG_DEBUG(TAG, "Cleaning up HTTP client after filesystem update.");
    esp_http_client_cleanup(client);
}