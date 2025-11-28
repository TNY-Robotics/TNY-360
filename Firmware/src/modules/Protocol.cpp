#include "modules/Protocol.hpp"
#include "modules/ProtocolHandlers.hpp"
#include "modules/Log.hpp"

namespace Protocol
{
    typedef struct PendingCall
    {
        uint16_t id;
        HandleResponseCallback callback;
    } PendingCall;

    PendingCall pending_calls[PROTOCOL_MAX_PENDING_CALLS] = {0};

    Error Handle(const Request& req, HandleResponseCallback callback)
    {
        if (!callback) {
            return Error::InvalidParameters;
        }

        // Find an empty slot for the pending call
        bool found = false;
        for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_CALLS; ++i) {
            if (pending_calls[i].callback == nullptr) {
                pending_calls[i].id = req.id;
                pending_calls[i].callback = callback;
                found = true;
                break;
            }
        }

        if (!found) // No empty slot found
        {
            return Error::NoMemory;
        }

        for (const auto& handler : Protocol::handlers) {
            if (handler.cmd == req.cmd && handler.len == req.len) {
                handler.callback(req);
                return Error::Ok;
            }
        }

        return Error::NotFound; // No matching handler found
    }

    Error Resolve(const Response& res)
    {
        // Find the pending call with the matching ID
        for (uint8_t i = 0; i < PROTOCOL_MAX_PENDING_CALLS; ++i) {
            if (pending_calls[i].id == res.id && pending_calls[i].callback != nullptr) {
                pending_calls[i].callback(res);
                
                pending_calls[i].id = 0;
                pending_calls[i].callback = nullptr;
                return Error::Ok;
            }
        }

        return Error::NotFound; // No matching pending call
    }
}
