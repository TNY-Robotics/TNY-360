#pragma once
#include "common/utils.hpp"
#include <freertos/FreeRTOS.h>

class DNSServer
{
public:
    DNSServer();

    /**
     * @brief Initialize the DNS server.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the DNS server.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    void __dns_server_task(void* parameter);

private:
    TaskHandle_t task_handle = nullptr;
    bool running = false;
    int socket_handle = -1;
};