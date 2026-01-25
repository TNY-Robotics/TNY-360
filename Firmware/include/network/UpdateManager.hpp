#pragma once
#include "common/utils.hpp"
#include "common/NVS.hpp"

class UpdateManager
{
public:
    constexpr static const char* TAG = "UpdateManager";

    UpdateManager();

    /**
     * @brief Initializes the Update Manager.
     * @return Error code indicating success or failure.
     * @note If an update was just applied, this function will trigger necessary verification steps, and may restart the system.
     */
    Error init();

    /**
     * @brief Deinitializes the Update Manager.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Checks for firmware updates from the server.
     * @return Error code indicating success or failure.
     */
    Error checkForUpdates();
    
    Error verifyFirmware();

    Error downloadAndApplyFirmwareUpdate();
    
    Error downloadAndApplyFilesystemUpdate();

    bool isUpdateAvailable() const { return updateAvailable; }

private:
    NVS::Handle* nvsHandle_ptr;

    bool updateAvailable = false;
    bool isFilesystemUpdatePending = false;
    char* latestVersion = nullptr;
    char* firmwareDownloadUrl = nullptr;
    char* filesystemDownloadUrl = nullptr;
};
