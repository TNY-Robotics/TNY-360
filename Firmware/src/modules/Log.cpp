#include <cstdint>
#include <cstring>
#include <cstdarg>
#include "modules/Log.hpp"
#include "config.hpp"

#ifdef DEBUG_MODE // Only include stdio if debugging is enabled
#include <cstdio>
#endif

Log::LineInfo logBuffer[LOG_MAX_LINES];
uint8_t logIndex = 0;
uint8_t logCount = 0;

void Log::Add(Log::Level level, const char* fmt, ...)
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
    // Log the message to serial for immediate feedback
    // This is useful for debugging during development

#ifdef DEBUG_MODE
    const char* levelStr = LevelToString(level);
    printf("[%s] %s\n", levelStr, message);
#endif
}

void Log::Add(Log::Level level, char* message)
{
    Log::Add(level, static_cast<const char*>(message));
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