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
    Log::Add(Log::Level::Info, TAG, "Initializing on FIRMWARE_VERSION: %s", FIRMWARE_VERSION);

    // Initialize and get NVS handle
    if (Error err = NVS::Init(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to initialize NVS: %d", static_cast<int>(err));
        return err;
    }
    if (Error err = NVS::Open("updt_mngr", &nvsHandle_ptr); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to open NVS namespace: %d", static_cast<int>(err));
        return err;
    }

    // Check if firmware needs to be verified on this boot
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to get OTA state for running partition");
        Log::Add(Log::Level::Error, TAG, "Rebooting in 3 seconds...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_restart();
        return Error::Unknown;
    }

    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
    {
        Log::Add(Log::Level::Info, TAG, "Verifying firmware update from last boot...");

        // Here you would add code to verify the firmware update.
        // For simplicity, we assume it's always successful.
        Error err = verifyFirmware();
        if (err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Firmware verification failed: %d", static_cast<int>(err));
            Log::Add(Log::Level::Info, TAG, "Reverting to previous firmware...");
            esp_ota_mark_app_invalid_rollback_and_reboot();
            return err;
        }
        
        Log::Add(Log::Level::Info, TAG, "Firmware verified successfully. Marking as valid.");
        esp_ota_mark_app_valid_cancel_rollback();

        Log::Add(Log::Level::Info, TAG, "Launching filesystem update");

        if (Error err = downloadAndApplyFilesystemUpdate(); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to apply pending filesystem update: %d", static_cast<int>(err));
            return err;
        }
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
    Log::Add(Log::Level::Debug, TAG, "Checking for updates...");

    esp_http_client_config_t config = {};
    config.url = OTA_FIRMWARE_LATEST_URL;
    config.cert_pem = (char *)root_ca_pem_start;
    config.timeout_ms = OTA_UPDATE_TIMEOUT_MS;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "x-robot-model", OTA_ROBOT_MODEL);

    if (esp_err_t err = esp_http_client_open(client, 0); err != ESP_OK)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to connect to API: %d", err);
        return Error::Unreachable;
    }

    esp_http_client_fetch_headers(client);
    
    char buffer[1024]; // Attention si ton JSON devient énorme
    int read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
    if (read_len > 0)
    {
        buffer[read_len] = 0; // Null terminate
        
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
                Log::Add(Log::Level::Debug, TAG, "Comparing version strings: %s vs %s", ver->valuestring, FIRMWARE_VERSION);
                if (strcmp(ver->valuestring, FIRMWARE_VERSION) != 0)
                {
                    Log::Add(Log::Level::Warning, TAG, "New version found: %s", ver->valuestring);
                    updateAvailable = true;
                    downloadAndApplyFirmwareUpdate(); // TODO : This should be done outside (menu validation)
                }
                else
                {
                    Log::Add(Log::Level::Info, TAG, "System is up to date.");
                    updateAvailable = false;
                }
            }
            cJSON_Delete(json);
        }
    }
    else
    {
        Log::Add(Log::Level::Error, TAG, "Failed to read response from API");
        return Error::Unknown;
    }

    return Error::None;
}

Error UpdateManager::downloadAndApplyFirmwareUpdate()
{
    if (!updateAvailable || !firmwareDownloadUrl)
    {
        Log::Add(Log::Level::Info, TAG, "No firmware update available to download.");
        return Error::InvalidState;
    }

    Log::Add(Log::Level::Info, TAG, "Starting firmware update from URL: %s", firmwareDownloadUrl);

    esp_http_client_config_t config = {};
    config.url = firmwareDownloadUrl;
    config.cert_pem = (char *)root_ca_pem_start;
    config.keep_alive_enable = true; // Keep the connection alive for large downloads
    config.timeout_ms = OTA_UPDATE_TIMEOUT_MS;

    esp_https_ota_config_t ota_config = {};
    ota_config.http_config = &config;

    if (esp_err_t ret = esp_https_ota(&ota_config); ret == ESP_OK)
    {
        // store filesystem URL in NVS for next boot to download
        if (Error err = nvsHandle_ptr->set("fs_updt_url", filesystemDownloadUrl); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to store filesystem update URL in NVS: %d", static_cast<int>(err));
            return err;
        }

        Log::Add(Log::Level::Info, TAG, "Firmware update applied successfully. Restarting in 3 seconds ...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_restart();
        return Error::None; // Not reached
    }
    else
    {
        Log::Add(Log::Level::Error, TAG, "Firmware update failed: %d", ret);
        return Error::SoftwareFailure;
    }
}

Error UpdateManager::downloadAndApplyFilesystemUpdate()
{
    if (!filesystemDownloadUrl)
    {
        Log::Add(Log::Level::Info, TAG, "No filesystem update URL, getting from NVS.");
        // get from NVS
        if (Error err = nvsHandle_ptr->get("fs_updt_url", filesystemDownloadUrl); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to get filesystem update URL from NVS: %d", static_cast<int>(err));
            return err;
        }
    }

    Log::Add(Log::Level::Info, TAG, "Starting filesystem update from URL: %s", filesystemDownloadUrl);

    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    if (!part) {
        Log::Add(Log::Level::Error, TAG, "Failed to find storage partition for filesystem update.");
        return Error::NotFound;
    }

    esp_http_client_config_t config = {};
    config.url = filesystemDownloadUrl;
    config.cert_pem = (char *)root_ca_pem_start;
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Open HTTP connection
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        Log::Add(Log::Level::Error, TAG, "Failed to open HTTP connection for filesystem update: %d", err);
        esp_http_client_cleanup(client);
        return Error::Unreachable;
    }

    // Get filesystem size
    int content_len = esp_http_client_fetch_headers(client);
    if (content_len > part->size) {
        Log::Add(Log::Level::Error, TAG, "Filesystem update size (%d) exceeds partition size (%d).", content_len, part->size);
        esp_http_client_cleanup(client);
        return Error::OutOfBounds;
    }

    // Erase partition
    err = esp_partition_erase_range(part, 0, part->size);
    if (err != ESP_OK) {
        Log::Add(Log::Level::Error, TAG, "Failed to erase storage partition: %d", err);
        esp_http_client_cleanup(client);
        return Error::HardwareFailure;
    }

    // Download and write filesystem data
    char buffer[OTA_UPDATE_FILESYSTEM_BUFFER_SIZE];
    size_t total_read = 0;

    while (true) {
        int read = esp_http_client_read(client, buffer, OTA_UPDATE_FILESYSTEM_BUFFER_SIZE);
        if (read < 0) {
            Log::Add(Log::Level::Error, TAG, "Error reading filesystem update data from HTTP.");
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
            Log::Add(Log::Level::Error, TAG, "Error writing filesystem update data to partition: %d", write_err);
            err = write_err;
            break;
        }
        total_read += read;
        Log::Add(Log::Level::Debug, TAG, "Written %d/%d bytes of filesystem update (%d%%).", total_read, content_len, (total_read * 100) / content_len);
    }

    if (err == ESP_OK) {
        Log::Add(Log::Level::Info, TAG, "Filesystem update applied successfully.");
        // Clear filesystem update URL from NVS
        nvsHandle_ptr->erase("fs_updt_url");
    } else {
        Log::Add(Log::Level::Error, TAG, "Filesystem update failed: %d", err);
        return Error::SoftwareFailure;
    }
    
    esp_http_client_cleanup(client);

    Log::Add(Log::Level::Info, TAG, "Rebooting in 3 seconds to apply filesystem update...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
    return Error::None;
}

Error UpdateManager::verifyFirmware()
{
    Log::Add(Log::Level::Info, TAG, "Running firmware diagnostics...");

    // TODO : Implement actual diagnostics here

    return Error::None;
}