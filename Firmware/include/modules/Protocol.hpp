#pragma once
#include "utils.hpp"

namespace Protocol
{
    typedef struct Request
    {
        uint16_t id; // 0-65535
        uint8_t cmd;
        uint8_t len;
        uint8_t* payload;
    } __attribute__((packed)) Request;
    
    typedef struct Response
    {
        uint16_t id; // 0-65535
        uint8_t status;
        uint8_t len;
        uint8_t* payload;
    } __attribute__((packed)) Response;

    using HandleResponseCallback = void(*)(const Response& res);

    /**
     * @brief Handle an incoming protocol request
     * @param req The incoming request
     * @param callback The callback to call when the response is received
     * @return Error Error code indicating success or failure
     */
    Error Handle(const Request& req, HandleResponseCallback callback);

    /**
     * @brief Resolve a pending protocol call with the given response
     * @param res The response to resolve the pending call with
     * @return Error Error code indicating success or failure
     */
    Error Resolve(const Response& res);
}