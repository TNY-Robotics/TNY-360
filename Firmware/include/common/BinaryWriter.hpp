#pragma once
#include "common/utils.hpp"
#include "common/Log.hpp"
#include <memory.h>

class BinaryWriter
{
public:
    BinaryWriter(uint8_t* buffer, size_t size)
        : buffer(buffer), size(size), offset(0) {}

    template<typename T>
    Error write(const T& value)
    {
        if (offset + sizeof(T) > size)
        {
            return Error::OutOfBounds;
        }
        memcpy(buffer + offset, &value, sizeof(T));
        offset += sizeof(T);
        return Error::None;
    }

    Error writeBytes(const uint8_t* data, size_t length)
    {
        if (offset + length > size)
        {
            return Error::OutOfBounds;
        }
        memcpy(buffer + offset, data, length);
        offset += length;
        return Error::None;
    }

    Error writeString(const char* str)
    {
        uint16_t strLength = (uint16_t)strlen(str);
        // write length first
        Error e = write(strLength);
        if (e != Error::None)
            return e;
        if (offset + strLength > size)
            return Error::OutOfBounds;
        return writeBytes((const uint8_t*)str, strLength);
    }

    size_t getOffset() const { return offset; }

private:
    uint8_t* buffer;
    size_t size;
    size_t offset;
};