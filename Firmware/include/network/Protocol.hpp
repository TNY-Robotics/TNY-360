#pragma once
#include "common/utils.hpp"
#include "common/config.hpp"

class Protocol
{
public:
    constexpr static const char* TAG = "Protocol";

    typedef struct Request
    {
        uint16_t id; // 0-65535
        uint8_t cmd;
        uint8_t len;
        uint8_t* payload;
        Request(uint16_t id = 0, uint8_t cmd = 0, uint8_t len = 0, uint8_t* payload = nullptr)
            : id(id), cmd(cmd), len(len), payload(payload) {}
    } __attribute__((packed)) Request;

    typedef struct Response
    {
        uint16_t id; // 0-65535
        uint8_t status;
        uint8_t* payload;
        uint8_t len;
        Response(uint16_t id = 0, uint8_t status = 0, uint8_t* payload = nullptr, uint8_t len = 0)
            : id(id), status(status), payload(payload), len(len) {}
    } __attribute__((packed)) Response;

    using HandleResponseCallback = void(*)(const Response& res);

    Protocol();

    /**
     * @brief Initialize the protocol handler
     * @return Error Error code indicating success or failure
     */
    Error init();

    /**
     * @brief Deinitialize the protocol handler
     * @return Error Error code indicating success or failure
     */
    Error deinit();

    /**
     * @brief Handle an incoming protocol request
     * @param req The incoming request
     * @param callback The callback to call when the response is received
     * @return Error Error code indicating success or failure
     */
    Error handleRequest(const Request& req, HandleResponseCallback callback);

    /**
     * @brief Resolve a pending protocol command with the given response
     * @param res The response to resolve the pending command with
     * @return Error Error code indicating success or failure
     */
    Error resolveCommand(const Response& res);

private:
    typedef struct PendingCommand
    {
        uint16_t id;
        HandleResponseCallback callback;
    } PendingCommand;

    PendingCommand pending_commands[PROTOCOL_MAX_PENDING_COMMANDS];
};