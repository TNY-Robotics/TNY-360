#pragma once
#include "common/utils.hpp"
#include "common/NVS.hpp"
#include <string>

class UpdateManager
{
public:
    constexpr static const char* TAG = "UpdateManager";

    enum class Status: uint8_t {
        Done = 0,
        FetchingUpdate,
        DownloadingFirmware,
        DownloadingFilesystem,
        UpdatingFirmware,
        UpdatingFilesystem,
        VerifyingFirmware,
        VerifyingFilesystem,
        Rebooting,
        ErrorUnreachable,
        ErrorInvalidJson,
        ErrorEmptyResponse,
        ErrorFirmwareUpdateFailed,
        ErrorFilesystemUpdateFailed,
        ErrorUnknown,
        ErrorPartitionNotFound,
        ErrorHTTPClient,
        ErrorOutOfBounds,
        ErrorEraseStorage,
    };

    UpdateManager();

    /**
     * @brief Initializes the Update Manager.
     * @return Error code indicating success or failure.
     * @note If an update was just applied, this function will trigger necessary verification steps, and may restart the system.
     */
    ::Status init();

    /**
     * @brief Deinitializes the Update Manager.
     * @return Error code indicating success or failure.
     */
    ::Status deinit();

    /**
     * @brief Get the current update status
     * @returns Current status of the update process (see Status enum)
     */
    Status getStatus();

    /**
     * @brief Get the current update progress
     * @returns Progress as a float between 0.0 and 1.0
     */
    float getProgress();

    /**
     * @brief Get the latest available version string from the update server
     * @returns Latest version string (e.g. "1.2.3") or empty string if not available
     */
    std::string getLatestVersion();

    /**
     * @brief Get if an update is currently available
     * @returns True if an update is available, false otherwise
     * @note Call checkForUpdate() and wait a bit to update this result
     */
    bool isUpdateAvailable();

    /**
     * @brief Get if an update is currently pending.
     * @returns True if an update is pending, false otherwise
     * @note If an update is pending, call continueUpdate() to continue the update process (e.g. after a reboot)
     */
    bool isUpdatePending();

    /**
     * @brief Checks for available updates on the update server. 
     * @returns Error code indicating success or failure of the update check process.
     * @note Use isUpdateAvailable() to check if an update was found after calling this function.
     */
    ::Status checkForUpdate();

    /**
     * @brief Starts the update process if an update is available.
     * @return Error code indicating success or failure of starting the update process.
     * @note This function will trigger the download and application of the update. Use getStatus() and getProgress() to monitor the update process.
     */
    ::Status startUpdate();

    /**
     * @brief Continues the update process if an update is pending (e.g. after a reboot).
     * @return Error code indicating success or failure of continuing the update process.
     * @note This function should be called only if isUpdatePending() returns true, to continue the update process after a reboot or interruption.
     */
    ::Status continueUpdate();

private:
    // For getters
    float progress = 0.0f;
    Status status = Status::Done;
    bool update_available = false;

    // Internal state
    std::string latest_version;
    std::string firmware_download_url;
    std::string filesystem_download_url;

    // Internal functions
    ::Status check_update();
    ::Status download_firmware();
    ::Status download_filesystem();
    ::Status verify_firmware();
};
