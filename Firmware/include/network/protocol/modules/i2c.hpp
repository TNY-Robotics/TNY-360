#pragma once
#include "network/protocol/Protocol.hpp"
#include <esp_system.h>
#include "common/I2C.hpp"
#include "common/BinaryReader.hpp"

namespace Protocol
{
namespace I2C
{
    constexpr uint8_t MODULE_ID = 0x0E;

    static void PingDevice(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        uint8_t address;
        if (reader.read(address) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }


        Error err = ::I2C::ProbeAddress(::I2C::handle_secondary, address);
        if (err == Error::NotFound)
        {
            bool found = false;
            ctx.respond(ResponseStatus::Ok, (const uint8_t*) &found, sizeof(found));
        }
        else if (err == Error::None)
        {
            bool found = true;
            ctx.respond(ResponseStatus::Ok, (const uint8_t*) &found, sizeof(found));
        }
        else
        {
            ctx.respond(ResponseStatus::UnknownError);
        }
    }

    static void WriteRegisters(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        uint8_t address;
        if (reader.read(address) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        
        uint8_t reg_address;
        if (reader.read(reg_address) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        
        uint8_t length;
        if (reader.read(length) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        Error err = ::I2C::WriteRegisters(::I2C::handle_secondary, address, reg_address, payload + 3, length);
        if (err != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok);
    }

    static void ReadRegisters(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        uint8_t address;
        if (reader.read(address) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        
        uint8_t reg_address;
        if (reader.read(reg_address) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        
        uint8_t length;
        if (reader.read(length) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        uint8_t* data = (uint8_t*)malloc(length);
        if (data == nullptr)
        {
            ctx.respond(ResponseStatus::OutOfMemory);
            return;
        }

        Error err = ::I2C::ReadRegisters(::I2C::handle_secondary, address, reg_address, data, length);
        if (err != Error::None)
        {
            free(data);
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        ctx.respond(ResponseStatus::Ok, data, length);
        free(data);
    }


    static ActionCallback actions[] = {
        PingDevice, // 0x00
        WriteRegisters, // 0x01
        ReadRegisters, // 0x02
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}