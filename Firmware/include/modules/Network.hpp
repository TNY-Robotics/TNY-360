#pragma once
#include "utils.hpp"

namespace Network
{
    /**
     * @brief Setup the device for STA mode
     * @note starts web server and web socket
     */
    Error SetupSTA();

    /**
     * @brief Setup the device for AP mode
     * @note starts web server, dns server and captive portal
     */
    Error SetupAP();
}