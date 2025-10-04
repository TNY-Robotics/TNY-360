#include "modules/Protocol.hpp"
#include "modules/Log.hpp"
#include "utils.hpp"

namespace Protocol
{
    using HandlerCallback = void(*)(const Request& req);

    typedef struct Handler
    {
        uint8_t cmd;
        uint8_t len;
        HandlerCallback callback;
    } Handler;

    static Handler handlers[] = {
        // ping command
        { 0x00, 0, [](const Request& req) {
            // Simply echo back the same payload
            Resolve(Response{
                .id = req.id,
                .status = true, // success
                .len = 0,
                .payload = nullptr
            });
        }},
        // set motor angle
        { 0x60, sizeof(uint8_t) + sizeof(float), [](const Request& req) {
            // Simply echo back the same payload
            Resolve(Response{
                .id = req.id,
                .status = true, // success
                .len = 0,
                .payload = nullptr
            });
        }},
    };
}
