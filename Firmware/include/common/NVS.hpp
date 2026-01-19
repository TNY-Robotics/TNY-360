#pragma once
#include "common/utils.hpp"

namespace NVS
{
    class Handle
    {
    public:
        virtual ~Handle() = default;

        template<typename T>
        Error get(const char* key, T& out_value)
        {
            size_t length = sizeof(T);
            return get(key, &out_value, &length);
        }

        template<typename T>
        Error set(const char* key, const T& value)
        {
            return set(key, &value, sizeof(T));
        }

        Error get(const char* key, void* out_value, size_t length)
        {
            size_t l = length;
            return get(key, out_value, &l);
        }

        virtual Error get(const char* key, void* out_value, size_t* length) = 0;
        virtual Error set(const char* key, const void* value, size_t length) = 0;
        virtual Error erase(const char* key) = 0;
    };

    Error Init();

    Error Open(const char* namespace_name, Handle** out_handle);
    void Close(Handle* handle);
}
