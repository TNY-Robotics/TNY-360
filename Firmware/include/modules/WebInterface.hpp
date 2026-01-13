#pragma once
#include "utils.hpp"

namespace WebInterface
{
    /**
     * @brief Initialize the web interface
     */
    Error Init();

    /**
     * @brief Start the web interface on specified port
     * @param port The port to listen on (default: 80)
     */
    Error Start(int port = 80);

    /**
     * @brief Stop the web interface
     */
    void Stop();

    /**
     * @brief Check if web interface is running
     * @return true if interface is running, false otherwise
     */
    bool IsRunning();

    void* _get_server_handle(); // For internal use
};
