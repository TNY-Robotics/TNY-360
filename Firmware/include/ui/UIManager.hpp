#pragma once
#include "common/utils.hpp"

class UIManager
{
public:
    constexpr static const char* TAG = "UIManager";

    UIManager();

    /**
     * @brief Initialize the ui system.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the ui system.
     * @return Error code indicating success or failure.
     */
    Error deinit();

private:
};
