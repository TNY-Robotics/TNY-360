#include <cstdint>
#include <cstring>
#include <cstdarg>
#include "common/Log.hpp"
#include "common/config.hpp"

#ifdef DEBUG_MODE // Only include stdio if debugging is enabled
#include <cstdio>
#endif

Log::LineInfo logBuffer[LOG_MAX_LINES];
uint8_t logIndex = 0;
uint8_t logCount = 0;
uint8_t logIndent = 0;

void Log::GroupStart()
{
    logIndent++;
}

void Log::GroupEnd()
{
    if (logIndent > 0)
        logIndent--;
}

void Log::Add(Log::Level level, const char* tag, const char* fmt, ...)
{
    char message[LOG_MAX_MSG_LEN];
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);
    
    // Store in circular buffer
    logBuffer[logIndex].level = level;
    strncpy(logBuffer[logIndex].message, message, LOG_MAX_MSG_LEN - 1);
    logBuffer[logIndex].message[LOG_MAX_MSG_LEN - 1] = '\0'; // Ensure null-termination
    
    logIndex = (logIndex + 1) % LOG_MAX_LINES;
    if (logCount < LOG_MAX_LINES) {
        logCount++;
    }
    
#if DEBUG_MODE == 1
    // Log the message to serial for immediate feedback
    // This is useful for debugging during development
    const char* levelStr = LevelToString(level);
    printf("[%s] [%s] %*s%s\n", levelStr, tag, logIndent * 2, "", message);
#endif
}

const Log::Level& Log::Get(uint8_t index)
{
    return logBuffer[index].level;
}

uint8_t Log::Count()
{
    return logCount;
}

const char* Log::LevelToString(Log::Level level)
{
    switch (level)
    {
        case Level::Info: return "INF";
        case Level::Warning: return "WRN";
        case Level::Error: return "ERR";
        case Level::Debug: return "DBG";
        default: return "---";
    }
}