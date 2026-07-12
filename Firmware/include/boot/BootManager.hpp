#pragma once
#include "common/utils.hpp"

namespace BootManager
{
    constexpr const char* TAG = "BootManager";

    /**
     * @brief Checks for special boot state, if any, runs the appropriate boot sequence.
     * @returns boolean true if special state is detected, else false.
     * @note If this function returns true, the standard booting sequence should be aborted.
     */
    bool CheckForSpecialBoot();

    /**
     * @brief Checks if the boot sequence should enter DIAGNOSTIC mode.
     * @note DIAGNOSTIC mode is used to perform system diagnostics and checks.
     * @returns boolean true if DIAGNOSTIC mode is needed, else false.
     */
    bool boot_DIAGNOSTIC_needed();

    /**
     * @brief Checks if the boot sequence should enter UPDATE mode.
     * @note UPDATE mode is used when the robot is booting after a firmware update. It ensures that the firmware is stable and properly installed.
     * @returns boolean true if UPDATE mode is needed, else false.
     */
    bool boot_UPDATE_needed();

    /**
     * @brief INTERNAL, boot sequence in DIAGNOSTIC mode
     * @note This function should only be called internally by the boot manager.
     */
    void boot_DIAGNOSTIC();

    /**
     * @brief INTERNAL, boot sequence in UPDATE mode
     * @note This function should only be called internally by the boot manager.
     */
    void boot_UPDATE();
}