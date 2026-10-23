#pragma once
#include "settings/config.hpp"
#include "common/utils.hpp"

namespace Settings
{
    static constexpr const char* TAG = "Settings";

    enum class RobotVersion
    {
        V1,
        V2,
        Unknown
    };

    /**
     * @brief Initialize the settings system.
     * @return Status code indicating success or failure.
     */
    Status Init();

    /**
     * @brief Deinitialize the settings system.
     * @return Status code indicating success or failure.
     */
    Status Deinit();

    /**
     * @brief Checks if the settings system is initialized.
     * @return True if initialized, false otherwise.
     */
    bool IsInitialized();

    /**
     * @brief Get the current robot version.
     * @return A RobotVersion enum value.
     */
    RobotVersion GetRobotVersion();

    /**
     * @brief Get the current robot configuration.
     * @return The current RobotConfig object.
     */
    RobotConfig GetConfig();

    /**
     * @brief Save the current configuration and reboot the system.
     * @note Reboot is needed to apply the new settings, don't try to apply them without rebooting, it will likely break things.
     * @return Status code indicating success or failure.
     */
    Status SaveAndReboot();
}