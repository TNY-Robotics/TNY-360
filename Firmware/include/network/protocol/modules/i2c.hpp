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

    /** <API_REF>
     * @module i2c 0x0E
     * @action pingDevice 0x00
     * @desc Pings an I2C device at the specified address to check if it is present on the bus.
     * @arg address uint8 I2C address of the device to ping.
     * @result found bool Whether a device responded at the specified address.
     * @impl done
     */
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

    /** <API_REF>
     * @module i2c 0x0E
     * @action writeRegisters 0x01
     * @desc Writes a sequence of bytes to a register of an I2C device.
     * @arg address uint8 I2C address of the device to write to.
     * @arg reg_address uint8 Register address to write to.
     * @arg length uint8 Number of bytes to write.
     * @arg data byte[] Data bytes to write to the register.
     * @impl done
     */
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

    /** <API_REF>
     * @module i2c 0x0E
     * @action readRegisters 0x02
     * @desc Reads a sequence of bytes from a register of an I2C device.
     * @arg address uint8 I2C address of the device to read from.
     * @arg reg_address uint8 Register address to read from.
     * @arg length uint8 Number of bytes to read.
     * @result data byte[] Read data bytes (of specified length).
     * @impl done
     */
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