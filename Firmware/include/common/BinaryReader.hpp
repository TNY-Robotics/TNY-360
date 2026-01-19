#pragma once
#include "common/utils.hpp"
#include <memory.h>

class BinaryReader
{
public:
    BinaryReader(const uint8_t* buffer, size_t size)
        : buffer(buffer), size(size), offset(0) {}

    template<typename T>
    Error read(T& out)
    {
        if (offset + sizeof(T) > size)
        {
            return Error::OutOfBounds;
        }
        out = *reinterpret_cast<const T*>(buffer + offset);
        offset += sizeof(T);
        return Error::None;
    }

    Error readBytes(uint8_t* out, size_t length)
    {
        if (offset + length > size)
        {
            return Error::OutOfBounds;
        }
        memcpy(out, buffer + offset, length);
        offset += length;
        return Error::None;
    }
private:
    const uint8_t* buffer;
    size_t size;
    size_t offset;
};