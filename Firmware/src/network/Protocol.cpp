#include "network/Protocol.hpp"
#include "network/ProtocolCommands.hpp"
#include "common/Log.hpp"

Protocol::Protocol()
{
}

Error Protocol::init()
{
    // Initialize pending commands array
    for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_COMMANDS; ++i) {
        pending_commands[i].id = 0;
        pending_commands[i].callback = nullptr;
    }

    return Error::None;
}

Error Protocol::deinit()
{
    return Error::None;
}

Error Protocol::handleRequest(const Request& req, HandleResponseCallback callback)
{
    if (!callback)
    {
        Log::Add(Log::Level::Error, TAG, "Invalid callback provided for request ID %d", req.id);
        return Error::InvalidParameters;
    }

    // Find a free slot in the pending commands array
    bool found = false;
    for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_COMMANDS; ++i) {
        if (pending_commands[i].callback == nullptr) {
            pending_commands[i].id = req.id;
            pending_commands[i].callback = callback;
            found = true;
            break;
        }
    }
    if (!found) // No empty slot found
    {
        Log::Add(Log::Level::Error, TAG, "No free slot available for pending command ID %d", req.id);
        return Error::NoMemory;
    }

    // Find the appropriate handler for the command
    // TODO : Instead of iterating through all handlers, use an array indexed by command ID (only 255 possible commands)
    CommandHandler* handler = nullptr;
    for (auto& h : handlers)
    {
        if (h.cmd == req.cmd)
        {
            handler = &h;
            break;
        }
    }
    if (handler == nullptr) // No handler found for this command
    {
        Log::Add(Log::Level::Error, TAG, "No handler found for command 0x%02X", req.cmd);
        return Error::NotFound;
    }

    // Check if the length of the payload matches the expected length
    if (handler->len != req.len)
    {
        Log::Add(Log::Level::Error, TAG, "Invalid payload length for command 0x%02X: expected %d, got %d", req.cmd, handler->len, req.len);
        return Error::InvalidParameters;
    }

    // Call the handler's callback and return success
    handler->callback(req, [](const Response& res) {
        Robot::GetInstance().getProtocol().resolveCommand(res);
    });
    return Error::None;
}

Error Protocol::resolveCommand(const Response& res)
{
    // Find the pending command with the matching ID
    for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_COMMANDS; ++i) {
        if (pending_commands[i].id == res.id) {
            // Call the associated callback
            if (pending_commands[i].callback) {
                pending_commands[i].callback(res);
            }
            else
            {
                Log::Add(Log::Level::Error, TAG, "No callback found for pending command ID %d", res.id);
                return Error::InvalidState;
            }
            // Clear the pending command slot
            pending_commands[i].id = 0;
            pending_commands[i].callback = nullptr;
            return Error::None;
        }
    }

    // No pending command found with the given ID
    return Error::NotFound;
}
