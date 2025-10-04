#pragma once
#include "utils.hpp"

namespace WebSocket
{
    /**
     * @brief Initialize the WebSocket server
     */
    Error Init();

    /**
     * @brief Attach the WebSocket handlers to the web server
     * This should be called after starting the web server
     */
    Error Attach();
}