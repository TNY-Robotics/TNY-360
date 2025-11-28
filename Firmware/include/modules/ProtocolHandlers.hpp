#include "modules/Protocol.hpp"
#include "modules/Log.hpp"
#include "modules/Motor.hpp"
#include "modules/LED.hpp"
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

    Handler handlers[] = {
        // ping command
        { 0x00, 0, [](const Request& req) {
            Resolve(Response{
                .id = req.id,
                .status = true, // success
                .len = 0,
                .payload = nullptr
            });
        }},
        // set motor target angle
        { 0x60, sizeof(uint8_t) + sizeof(float) + sizeof(float), [](const Request& req) {
            Error err = Motor::SetAngle(
                static_cast<Motor::Id>(req.payload[0]),
                *((float*)(req.payload + 1)),
                *((float*)(req.payload + 5))
            );
            Resolve(Response{
                .id = req.id,
                .status = (err == Error::Ok),
                .len = 0,
                .payload = nullptr
            });
        }},
        // set motor target force
        { 0x61, sizeof(uint8_t) + sizeof(uint8_t), [](const Request& req) {
            Error err = Motor::SetForce(
                static_cast<Motor::Id>(req.payload[0]),
                static_cast<Motor::ForceLevel>(req.payload[1])
            );
            Resolve(Response{
                .id = req.id,
                .status = (err == Error::Ok),
                .len = 0,
                .payload = nullptr
            });
        }},
        // set LED target color
        { 0x62, sizeof(uint8_t) + sizeof(uint8_t) * 3 + sizeof(float), [](const Request& req) {
            Error err = LED::SetColor(
                static_cast<LED::Id>(req.payload[0]),
                LED::Color(req.payload[1], req.payload[2], req.payload[3]),
                *((float*)(req.payload + 4))
            );
            Resolve(Response{
                .id = req.id,
                .status = (err == Error::Ok),
                .len = 0,
                .payload = nullptr
            });
        }},
        // get motor current angle
        { 0x20, sizeof(uint8_t), [](const Request& req) {
            float angle = 0.0f;
            Error err = Motor::GetCurrentAngle(
                static_cast<Motor::Id>(req.payload[0]),
                &angle
            );
            Resolve(Response{
                .id = req.id,
                .status = (err == Error::Ok),
                .len = sizeof(float),
                .payload = (uint8_t*)&angle
            });
        }},
        // get motor target angle
        { 0x21, sizeof(uint8_t), [](const Request& req) {
            float angle = 0.0f;
            Error err = Motor::GetTargetAngle(
                static_cast<Motor::Id>(req.payload[0]),
                &angle
            );
            Resolve(Response{
                .id = req.id,
                .status = (err == Error::Ok),
                .len = sizeof(float),
                .payload = (uint8_t*)&angle
            });
        }},
    };
}
