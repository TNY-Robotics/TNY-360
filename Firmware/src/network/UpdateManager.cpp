#include "network/UpdateManager.hpp"
#include "esp_http_client.h"
#include "esp_partition.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "ArduinoJson.hpp"

// Code-embeded root CA certificate (PEM format)
extern const uint8_t root_ca_pem_start[] asm("_binary_root_ca_pem_start");
extern const uint8_t root_ca_pem_end[]   asm("_binary_root_ca_pem_end");

UpdateManager::UpdateManager()
{
}

Status UpdateManager::init()
{
    LOG_SCOPE(TAG, "UpdateManager::init");

    return ::Status::Ok;
}

Status UpdateManager::deinit()
{
    return ::Status::Ok;
}

UpdateManager::Status UpdateManager::getStatus()
{
    return status;
}

float UpdateManager::getProgress()
{
    return progress;
}

std::string UpdateManager::getLatestVersion()
{
    return latest_version;
}

bool UpdateManager::isUpdateAvailable()
{
    return update_available;
}

bool UpdateManager::isUpdatePending()
{
    // Using ota state to determine if an update is pending
    esp_ota_img_states_t ota_state;
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    
    if (esp_err_t err = esp_ota_get_state_partition(running_partition, &ota_state); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to get OTA state for running partition");
        return false;
    }
    return ota_state == ESP_OTA_IMG_PENDING_VERIFY;
}

Status UpdateManager::checkForUpdate()
{
    // Note : Starting in an other task to avoid blocking
    BaseType_t ret = xTaskCreatePinnedToCore([](void* param) {
        UpdateManager* self = static_cast<UpdateManager*>(param);
        // First download the firmware
        self->check_update();

        vTaskDelete(nullptr);
    }, "UpdateTask", 8192, this, tskIDLE_PRIORITY + 1, nullptr, CORE_BRAIN);

    if (ret != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create update task");
        return ::Status::Failure;
    }
    return ::Status::Ok;
}

Status UpdateManager::startUpdate()
{
    // Note : Starting in an other task to avoid blocking
    BaseType_t ret = xTaskCreatePinnedToCore([](void* param) {
        UpdateManager* self = static_cast<UpdateManager*>(param);
        // First download the firmware
        if (self->download_firmware() != ::Status::Ok)
        {
            LOG_ERROR(TAG, "Firmware update failed, aborting update process");
            self->status = Status::ErrorFirmwareUpdateFailed;
            vTaskDelete(nullptr);
            return;
        }
        // Then download the filesystem
        if (self->download_filesystem() != ::Status::Ok)
        {
            LOG_ERROR(TAG, "Filesystem update failed, aborting update process");
            self->status = Status::ErrorFilesystemUpdateFailed;
            vTaskDelete(nullptr);
            return;
        }
        LOG_INFO(TAG, "Update process completed successfully, rebooting to apply update...");
        // Ready to reboot
        self->status = Status::Rebooting;
        // Small delay to ensure message is displayed
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
        vTaskDelete(nullptr); // not needed (we reboot) but why not
    }, "UpdateTask", 8192, this, tskIDLE_PRIORITY + 1, nullptr, CORE_BRAIN);

    if (ret != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create update task");
        return ::Status::Failure;
    }
    return ::Status::Ok;
}

Status UpdateManager::continueUpdate()
{
    if (!isUpdatePending())
    {
        LOG_ERROR(TAG, "No update is pending, cannot continue update process");
        return ::Status::InvalidState;
    }

    // Well ... we just need to verify if everything is working
    if (::Status err = verify_firmware(); err != ::Status::Ok)
    {
        LOG_ERROR(TAG, "Firmware verification failed after update");
        esp_ota_mark_app_invalid_rollback_and_reboot();
        return err;
    }

    // Mark the update as valid, so it won't be rolled back
    LOG_INFO(TAG, "Firmware verification successful, marking update as valid...");
    if (esp_err_t err = esp_ota_mark_app_valid_cancel_rollback(); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to mark updated firmware as valid: %d", err);
        return ::Status::Failure;
    }
    return ::Status::Ok;
}


/// ========== Internal functions ========== ///


Status UpdateManager::check_update()
{
    status = Status::FetchingUpdate;

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
        return ::Status::Failure;
    }

    esp_http_client_fetch_headers(client);
    
    char buffer[1024];
    int read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
    if (read_len <= 0)
    {
        LOG_ERROR(TAG, "Failed to read response from API");
        status = Status::ErrorEmptyResponse;
        return ::Status::Unknown;
    }
    
    buffer[read_len] = 0; // Null terminate for safety

    ArduinoJson::JsonDocument json;
    ArduinoJson::DeserializationError err = ArduinoJson::deserializeJson(json, buffer);
    if (err != ArduinoJson::DeserializationError::Ok)
    {
        LOG_ERROR(TAG, "Failed to parse JSON response: %s", err.c_str());
        status = Status::ErrorInvalidJson;
        return ::Status::InvalidState;
    }

    const char *ver = json["version"];
    const char *fw_url = json["firmwareDownloadUrl"];
    const char *fs_url = json["filesystemDownloadUrl"];

    if (!ver || !fw_url || !fs_url)
    {
        LOG_ERROR(TAG, "API response is missing required fields");
        status = Status::ErrorInvalidJson;
        return ::Status::Unknown;
    }

    // copy in member variables
    latest_version = ver;
    firmware_download_url = fw_url;
    filesystem_download_url = fs_url;
    
    json.clear(); // Free JSON document memory

    LOG_DEBUG(TAG, "Comparing version strings: %s vs %s", ver, FIRMWARE_VERSION);
    if (latest_version.compare(FIRMWARE_VERSION) != 0) // if not matching, probably higher.
    {
        LOG_DEBUG(TAG, "New version found: %s", ver);
        update_available = true;
    }
    else
    {
        LOG_DEBUG(TAG, "System is up to date.");
        update_available = false;
    }
    status = Status::Done;

    return ::Status::Unknown;
}

Status UpdateManager::download_firmware()
{
    if (firmware_download_url.empty())
    {
        LOG_ERROR(TAG, "Firmware download URL is empty");
        return ::Status::InvalidState;
    }

    status = Status::DownloadingFirmware;
    progress = 0.0f;

    LOG_DEBUG(TAG, "Starting firmware update from URL: %s", firmware_download_url);
    esp_http_client_config_t config = {};
    config.url = firmware_download_url.c_str();
    config.cert_pem = (char *)root_ca_pem_start;
    config.keep_alive_enable = true;
    config.timeout_ms = OTA_UPDATE_TIMEOUT_MS;

    esp_https_ota_config_t ota_config = {};
    ota_config.http_config = &config;

    // 1. Initialize OTA session
    esp_https_ota_handle_t https_ota_handle = nullptr;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK)
    {
        LOG_ERROR(TAG, "ESP HTTPS OTA Begin failed: %d", err);
        status = Status::ErrorFirmwareUpdateFailed;
        return ::Status::Failure;
    }

    status = Status::UpdatingFirmware;
    LOG_INFO(TAG, "OTA session started, downloading...");

    // 2. Loop for downloading and writing firmware in chunks, while updating progress
    while (true)
    {
        err = esp_https_ota_perform(https_ota_handle);
        
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
            break; // Download is either complete or failed

        // Update progress
        int len_read = esp_https_ota_get_image_len_read(https_ota_handle);
        int total_len = esp_https_ota_get_image_size(https_ota_handle);

        if (total_len > 0)
        {
            progress = static_cast<float>(len_read) / static_cast<float>(total_len);
        }
    }

    // 3. Verify if the data is correct
    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true)
    {
        LOG_ERROR(TAG, "Complete data was not received.");
        esp_https_ota_abort(https_ota_handle); // Cancel OTA session and free resources
        status = Status::ErrorFirmwareUpdateFailed;
        return ::Status::Failure;
    }

    // 4. Finishing OTA update
    err = esp_https_ota_finish(https_ota_handle);
    if (err != ESP_OK)
    {
        LOG_ERROR(TAG, "Firmware update finish failed: %d", err);
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            LOG_ERROR(TAG, "Image validation failed, image is corrupted.");
        }
        status = Status::ErrorFirmwareUpdateFailed;
        return ::Status::Failure;
    }
    
    LOG_INFO(TAG, "Firmware update applied successfully.");
    
    status = Status::Done;
    return ::Status::Ok;
}

Status UpdateManager::download_filesystem()
{
    if (filesystem_download_url.empty())
    {
        LOG_ERROR(TAG, "Filesystem download URL is empty");
        return ::Status::InvalidState;
    }

    status = Status::DownloadingFilesystem;
    progress = 0.0f;

    LOG_DEBUG(TAG, "Starting filesystem update from URL: %s", filesystem_download_url.c_str());

    // 1. Find Partition
    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    if (!part) {
        LOG_ERROR(TAG, "Failed to find storage partition.");
        status = Status::ErrorPartitionNotFound;
        return ::Status::Failure; 
    }
    LOG_DEBUG(TAG, "Storage partition found: addr 0x%X, size %d bytes.", part->address, part->size);

    // 2. Setup HTTP Client
    esp_http_client_config_t config = {};
    config.url = filesystem_download_url.c_str();
    config.cert_pem = (char *)root_ca_pem_start;
    config.timeout_ms = OTA_UPDATE_TIMEOUT_MS;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        LOG_ERROR(TAG, "Failed to initialize HTTP client.");
        status = Status::ErrorHTTPClient;
        return ::Status::Failure;
    }

    // Auto-cleanup macro for the client (RAII-like pattern)
    // Ensures client is cleaned up even if we return early
    struct ClientCleaner {
        esp_http_client_handle_t c;
        ~ClientCleaner() { if (c) esp_http_client_cleanup(c); }
    } cleaner{client};

    // 3. Open Connection & Fetch Headers
    if (esp_err_t err = esp_http_client_open(client, 0); err != ESP_OK) {
        LOG_ERROR(TAG, "Failed to open HTTP connection: %d", err);
        status = Status::ErrorUnreachable;
        return ::Status::Failure;
    }

    int content_len = esp_http_client_fetch_headers(client);
    if (content_len <= 0) {
        LOG_ERROR(TAG, "Invalid content length from server: %d", content_len);
        status = Status::ErrorEmptyResponse;
        return ::Status::Failure;
    }

    // 4. Verify Size constraints
    if (content_len > part->size) {
        LOG_ERROR(TAG, "Update size (%d) exceeds partition size (%d).", content_len, part->size);
        status = Status::ErrorOutOfBounds;
        return ::Status::InvalidState;
    }
    
    // 5. Erase Partition
    LOG_INFO(TAG, "Erasing storage partition (size: %d bytes)...", part->size);
    if (esp_err_t err = esp_partition_erase_range(part, 0, part->size); err != ESP_OK) {
        LOG_ERROR(TAG, "Failed to erase storage partition: %d", err);
        status = Status::ErrorEraseStorage;
        return ::Status::Failure;
    }

    // 6. Download and Write Loop
    status = Status::UpdatingFilesystem;
    char buffer[OTA_UPDATE_FILESYSTEM_BUFFER_SIZE];
    size_t total_read = 0;
    esp_err_t write_err = ESP_OK;

    LOG_INFO(TAG, "Downloading and writing filesystem...");
    
    while (total_read < content_len) {
        int read_len = esp_http_client_read(client, buffer, sizeof(buffer));
        
        if (read_len < 0) {
            LOG_ERROR(TAG, "Error reading from HTTP stream.");
            status = Status::ErrorFilesystemUpdateFailed;
            return ::Status::Failure;
        } else if (read_len == 0) {
            LOG_WARNING(TAG, "Connection closed prematurely by server.");
            break; 
        }

        write_err = esp_partition_write(part, total_read, buffer, read_len);
        if (write_err != ESP_OK) {
            LOG_ERROR(TAG, "Failed to write to partition at offset %d: %d", total_read, write_err);
            status = Status::ErrorFilesystemUpdateFailed;
            return ::Status::Failure;
        }

        total_read += read_len;
        progress = static_cast<float>(total_read) / static_cast<float>(content_len);
    }

    // 7. Final Validation
    if (total_read != content_len || write_err != ESP_OK) {
        LOG_ERROR(TAG, "Filesystem update incomplete. Expected %d, got %d bytes.", content_len, total_read);
        status = Status::ErrorFilesystemUpdateFailed;
        return ::Status::Failure;
    }

    LOG_INFO(TAG, "Filesystem update completed successfully.");
    status = Status::Done;
    
    return ::Status::Ok;
}