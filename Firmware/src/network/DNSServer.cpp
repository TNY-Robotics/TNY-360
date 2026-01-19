#include "network/DNSServer.hpp"
#include "common/Log.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <esp_netif.h>
#include <memory.h>
#include <cstring>

constexpr const char* TAG = "DNSServer";

void create_dns_response(uint8_t* query, size_t query_len, uint8_t* response, size_t* response_len)
{
    // Copy query header
    memcpy(response, query, 12);
    
    // Set response flags
    response[2] = 0x81; // Response, authoritative
    response[3] = 0x80; // No error
    
    // Copy question section
    size_t question_len = query_len - 12;
    memcpy(response + 12, query + 12, question_len);
    
    // Add answer section
    size_t answer_offset = 12 + question_len;
    
    // Name (pointer to question)
    response[answer_offset] = 0xC0;
    response[answer_offset + 1] = 0x0C;
    
    // Type A
    response[answer_offset + 2] = 0x00;
    response[answer_offset + 3] = 0x01;
    
    // Class IN
    response[answer_offset + 4] = 0x00;
    response[answer_offset + 5] = 0x01;
    
    // TTL (1 hour)
    response[answer_offset + 6] = 0x00;
    response[answer_offset + 7] = 0x00;
    response[answer_offset + 8] = 0x0E;
    response[answer_offset + 9] = 0x10;
    
    // Data length (4 bytes for IPv4)
    response[answer_offset + 10] = 0x00;
    response[answer_offset + 11] = 0x04;
    
    // IP address (192.168.4.1 - default AP IP)
    response[answer_offset + 12] = 192;
    response[answer_offset + 13] = 168;
    response[answer_offset + 14] = 4;
    response[answer_offset + 15] = 1;
    
    // Set answer count
    response[6] = 0x00;
    response[7] = 0x01;
    
    *response_len = answer_offset + 16;
}

void DNSServer::__dns_server_task(void* parameter)
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    uint8_t buffer[512];
    uint8_t response[512];
    size_t response_len;
    
    socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_handle < 0) {
        Log::Add(Log::Level::Error, TAG, "Failed to create socket");
        vTaskDelete(nullptr);
        return;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(socket_handle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(socket_handle, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        Log::Add(Log::Level::Error, TAG, "Failed to bind socket");
        close(socket_handle);
        socket_handle = -1;
        vTaskDelete(nullptr);
        return;
    }
            
    while (running) {
        int len = recvfrom(socket_handle, buffer, sizeof(buffer), 0, 
                        (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (len > 0 && len >= 12) { // Minimum DNS header size
            // Create response that redirects all queries to our AP IP
            create_dns_response(buffer, len, response, &response_len);
            
            sendto(socket_handle, response, response_len, 0, 
                (struct sockaddr*)&client_addr, client_addr_len);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    if (socket_handle >= 0) {
        close(socket_handle);
        socket_handle = -1;
    }
    
    task_handle = nullptr;
    vTaskDelete(nullptr);
}

void server_task(void* parameter)
{
    DNSServer* dns_server = static_cast<DNSServer*>(parameter);
    dns_server->__dns_server_task(parameter);
}

DNSServer::DNSServer()
{
}

Error DNSServer::init()
{
    if (xTaskCreate(server_task, "dns_server", 4096, this, 5, &task_handle) != pdPASS)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to create DNS server task");
        return Error::SoftwareFailure;
    }

    return Error::None;
}

Error DNSServer::deinit()
{
    running = false;
    if (task_handle)
    {
        vTaskDelete(task_handle);
        task_handle = nullptr;
    }

    return Error::None;
}