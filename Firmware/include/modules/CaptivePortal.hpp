#pragma once
#include "utils.hpp"

namespace CaptivePortal
{
    /**
     * @brief Initialize the captive portal (load website content)
     */
    Error Init();

    /**
     * @brief Attach the captive portal handlers to the web server
     * This should be called after starting the web server
     */
    Error Attach();
}