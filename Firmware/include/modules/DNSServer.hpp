#pragma once
#include "utils.hpp"

namespace DNSServer
{
    /**
     * @brief Initialize DNS server for captive portal
     */
    Error Init();

    /**
     * @brief Start DNS server
     * @param port DNS port (default: 53)
     */
    Error Start(int port = 53);

    /**
     * @brief Stop DNS server
     */
    Error Stop();

    /**
     * @brief Check if DNS server is running
     * @return true if running, false otherwise
     */
    bool IsRunning();
}