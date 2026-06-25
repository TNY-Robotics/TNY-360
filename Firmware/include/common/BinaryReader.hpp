#pragma once
#include "common/utils.hpp"
#include <memory.h>

class BinaryReader
{
public:
    BinaryReader(const uint8_t* buffer, size_t size)
        : buffer(buffer), size(size), offset(0) {}

    template<typename T>
    Status read(T& out)
    {
        if (offset + sizeof(T) > size)
        {
            return Status::OutOfBounds;
        }
        out = *reinterpret_cast<const T*>(buffer + offset);
        offset += sizeof(T);
        return Status::Ok;
    }

    Status readBytes(uint8_t* out, size_t length)
    {
        if (offset + length > size)
        {
            return Status::OutOfBounds;
        }
        memcpy(out, buffer + offset, length);
        offset += length;
        return Status::Ok;
    }

    Status readString(char* out, size_t maxLength)
    {
        uint16_t strLength;
        if (read(strLength) != Status::Ok)
        {
            return Status::OutOfBounds;
        }

        if (strLength >= maxLength)
        {
            return Status::OutOfBounds;
        }

        return readBytes((uint8_t*) out, strLength);
    }

private:
    const uint8_t* buffer;
    size_t size;
    size_t offset;
};