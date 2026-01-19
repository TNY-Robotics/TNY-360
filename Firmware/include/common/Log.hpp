#pragma once
#include <cstdint>
#include "config.hpp"

namespace Log
{
    enum class Level : uint8_t
    {
        Info,
        Warning,
        Error,
        Debug
    };

    struct LineInfo
    {
        Level level;
        char message[LOG_MAX_MSG_LEN];
    };

    void GroupStart();
    void GroupEnd();

    void Add(Level level, const char* tag, const char* fmt, ...);
    const Level& Get(uint8_t index);
    uint8_t Count();

    const char* LevelToString(Level level);
}