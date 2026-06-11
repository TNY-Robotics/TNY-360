#pragma once
#include <cstdint>
#include "config.hpp"
#include "esp_timer.h"

namespace Log
{
    enum class Level : uint8_t
    {
        Info,
        Warning,
        Error,
        Debug,
        Success
    };

    struct LineInfo
    {
        /// @brief Timestamp in milliseconds since boot
        uint32_t timestampMs;
        /// @brief Level of the log message (see Log::Level)
        Level level;
        /// @brief Indentation level for grouping log messages (used by LOG_SCOPE)
        uint8_t indent;
        /// @brief Tag of the log message (e.g. module name)
        char tag[LOG_MAX_TAG_LEN];
        /// @brief The log message content (as a null-terminated string)
        char message[LOG_MAX_MSG_LEN];
    };

    /// @brief Start a new log group, increasing the indentation level for subsequent log messages until GroupEnd is called.
    void GroupStart();
    /// @brief End the current log group, decreasing the indentation level for subsequent log messages.
    void GroupEnd();

    /**
     * @brief Get a log line by its index, where 0 is the most recent log, 1 is the previous log, and so on.
     * @param index The index of the log line to retrieve (0 for most recent)
     * @return A reference to the LineInfo struct containing the log line information.
     * @note If the index is out of bounds (greater than or equal to the number of stored logs), an empty line with level Info and empty message will be returned.
     */
    const LineInfo& GetLine(uint16_t index);

    /**
     * @brief Get the total number of log lines currently stored in the buffer (up to LOG_MAX_LINES).
     * @return The number of log lines currently stored.
     */
    uint16_t Count();

    /// @brief Convert a Log::Level enum value to a human-readable string (e.g. "INF" for Info).
    const char* LevelToString(Level level);

    class Scope
    {
    public:
        Scope();
        Scope(const char* TAG, const char* fmt, ...);
        ~Scope();

        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
    };

    namespace internal
    {
        void Add(Level level, const char* tag, const char* fmt, ...);
        void AddV(Level level, const char* tag, const char* fmt, va_list args);
    }
}

#define LOG_INFO(tag, fmt, ...)    Log::internal::Add(Log::Level::Info, tag, fmt, ##__VA_ARGS__)
#define LOG_WARNING(tag, fmt, ...) Log::internal::Add(Log::Level::Warning, tag, fmt, ##__VA_ARGS__)
#define LOG_ERROR(tag, fmt, ...)   Log::internal::Add(Log::Level::Error, tag, fmt, ##__VA_ARGS__)
#define LOG_SUCCESS(tag, fmt, ...) Log::internal::Add(Log::Level::Success, tag, fmt, ##__VA_ARGS__)

#ifdef RELEASE_MODE
    #define LOG_DEBUG(tag, fmt, ...) do {} while(0)
    #define LOG_SCOPE(tag, fmt, ...) do {} while(0)
#else
    #define LOG_DEBUG(tag, fmt, ...) Log::internal::Add(Log::Level::Debug, tag, fmt, ##__VA_ARGS__)
    #define LOG_SCOPE(tag, fmt, ...) Log::Scope _log_scope_instance(tag, fmt, ##__VA_ARGS__)
#endif