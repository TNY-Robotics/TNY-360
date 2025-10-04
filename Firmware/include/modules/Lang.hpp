#pragma once
#include <cstdint>

namespace Lang
{
    enum class KEY: uint32_t
    {
        HELLO,
        LONG,
    };

    bool Init();
    const char* Get(KEY key);
}