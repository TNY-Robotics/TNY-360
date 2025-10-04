#pragma once
#include "utils.hpp"

namespace WebServer
{
    /**
     * @brief Initialize the web server
     */
    Error Init();

    /**
     * @brief Start the web server on specified port
     * @param port The port to listen on (default: 80)
     */
    Error Start(int port = 80);

    /**
     * @brief Stop the web server
     */
    void Stop();

    /**
     * @brief Check if web server is running
     * @return true if server is running, false otherwise
     */
    bool IsRunning();

    void* _get_server_handle(); // For internal use
}