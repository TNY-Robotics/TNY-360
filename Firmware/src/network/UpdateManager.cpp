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
    Log::Add(Log::Level::Debug, TAG, "Initializing on FIRMWARE_VERSION: %s", FIRMWARE_VERSION);

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
    Log::Add(Log::Level::Debug, TAG, "Checking if firmware verification is needed...");
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

    Log::Add(Log::Level::Debug, TAG, "OTA info: partition label=%s, type=%d, subtype=%d, state=%d", running->label, running->type, running->subtype, static_cast<uint32_t>(ota_state));

    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
    {
        Log::Add(Log::Level::Debug, TAG, "Firmware verification is needed.");

        // Here you would add code to verify the firmware update.
        // For simplicity, we assume it's always successful.
        Error err = verifyFirmware();
        if (err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Firmware verification failed: %d", static_cast<int>(err));
            Log::Add(Log::Level::Error, TAG, "Reverting to previous firmware...");
            esp_ota_mark_app_invalid_rollback_and_reboot();
            return err;
        }
        
        Log::Add(Log::Level::Debug, TAG, "No error during diagnostics, marking firmware as valid.");
        esp_ota_mark_app_valid_cancel_rollback();

        isFilesystemUpdatePending = true; // mark filesystem update as pending
        // so we download the new one once connected to AP (checkForUpdate method)
    }
    else
    {
        Log::Add(Log::Level::Debug, TAG, "No firmware verification needed on this boot.");
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
    if (strcmp(FIRMWARE_VERSION, "0.0.0") == 0)
    {
        Log::Add(Log::Level::Debug, TAG, "Firmware version is 0.0.0 (BETA, WIP, UNRELEASED), skipping update check.");
        return Error::None;
    }

    Log::Add(Log::Level::Debug, TAG, "Checking for updates...");

    // check the current filesystem version, if older than firmware version, download it

    if (isFilesystemUpdatePending)
    {
        Log::Add(Log::Level::Debug, TAG, "Detected pending filesystem update, getting filesystem update URL from NVS.");
        if (Error err = nvsHandle_ptr->get("fs_updt_url", filesystemDownloadUrl); err == Error::None)
        {
            if (Error err = downloadAndApplyFilesystemUpdate(); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to apply pending filesystem update: %d", static_cast<int>(err));
                return err;
            }

            // remove fs_updt_url key to indicate update is done
            if (Error err = nvsHandle_ptr->erase("fs_updt_url"); err != Error::None && err != Error::NotFound)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to clear filesystem update URL from NVS: %s", ErrorToString(err));
                return err;
            }
            isFilesystemUpdatePending = false;
            
            Log::Add(Log::Level::Debug, TAG, "Rebooting in 3 seconds to apply filesystem update...");
            vTaskDelay(pdMS_TO_TICKS(3000));
            esp_restart();
        }
        else
        {
            isFilesystemUpdatePending = false;
            Log::Add(Log::Level::Debug, TAG, "Couldn't get filesystem update URL from NVS: %d", static_cast<int>(err));
        }
    }

    Log::Add(Log::Level::Debug, TAG, "Contacting update server at %s", OTA_FIRMWARE_LATEST_URL);
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
                    Log::Add(Log::Level::Debug, TAG, "New version found: %s", ver->valuestring);
                    updateAvailable = true;
                    // NOTE : Not launching update here, just marking it as available
                    // It will be triggered by the user if desired
                }
                else
                {
                    Log::Add(Log::Level::Debug, TAG, "System is up to date.");
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
        Log::Add(Log::Level::Debug, TAG, "No firmware update available to download.");
        return Error::InvalidState;
    }

    Log::Add(Log::Level::Debug, TAG, "Starting firmware update from URL: %s", firmwareDownloadUrl);
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
        Log::Add(Log::Level::Debug, TAG, "No filesystem update URL, getting from NVS.");
        // get from NVS
        if (Error err = nvsHandle_ptr->get("fs_updt_url", filesystemDownloadUrl); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to get filesystem update URL from NVS: %d", static_cast<int>(err));
            return err;
        }
    }

    Log::Add(Log::Level::Debug, TAG, "Starting filesystem update from URL: %s", filesystemDownloadUrl);

    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    if (!part) {
        Log::Add(Log::Level::Error, TAG, "Failed to find storage partition for filesystem update.");
        return Error::NotFound;
    }

    Log::Add(Log::Level::Debug, TAG, "Found storage partition at address 0x%X, size %d bytes.", part->address, part->size);
    esp_http_client_config_t config = {};
    config.url = filesystemDownloadUrl;
    config.cert_pem = (char *)root_ca_pem_start;
    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (!client) {
        Log::Add(Log::Level::Error, TAG, "Failed to initialize HTTP client for filesystem update.");
        return Error::Unknown;
    }
    Log::Add(Log::Level::Debug, TAG, "HTTP client initialized for filesystem update.");

    // Open HTTP connection
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        Log::Add(Log::Level::Error, TAG, "Failed to open HTTP connection for filesystem update: %d", err);
        esp_http_client_cleanup(client);
        return Error::Unreachable;
    }
    
    Log::Add(Log::Level::Debug, TAG, "HTTP connection opened for filesystem update.");

    // Get filesystem size
    int content_len = esp_http_client_fetch_headers(client);
    if (content_len > part->size) {
        Log::Add(Log::Level::Error, TAG, "Filesystem update size (%d) exceeds partition size (%d).", content_len, part->size);
        esp_http_client_cleanup(client);
        return Error::OutOfBounds;
    }

    Log::Add(Log::Level::Debug, TAG, "Filesystem update size: %d bytes.", content_len);

    // Erase partition
    err = esp_partition_erase_range(part, 0, part->size);
    if (err != ESP_OK) {
        Log::Add(Log::Level::Error, TAG, "Failed to erase storage partition: %d", err);
        esp_http_client_cleanup(client);
        return Error::HardwareFailure;
    }

    Log::Add(Log::Level::Debug, TAG, "Storage partition erased successfully.");

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
    
    Log::Add(Log::Level::Debug, TAG, "Cleaning up HTTP client after filesystem update.");
    esp_http_client_cleanup(client);
    return Error::None;
}

Error UpdateManager::verifyFirmware()
{
    Log::Add(Log::Level::Info, TAG, "Running firmware diagnostics...");

    // TODO : Implement actual diagnostics here

    Log::Add(Log::Level::Info, TAG, "Firmware diagnostics completed successfully.");
    return Error::None;
}