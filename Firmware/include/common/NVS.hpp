#pragma once
#include "common/utils.hpp"

namespace NVS
{
    class Handle
    {
    public:
        virtual ~Handle() = default;

        template<typename T>
        Status get(const char* key, T& out_value)
        {
            size_t length = sizeof(T);
            return get(key, &out_value, &length);
        }

        template<typename T>
        Status set(const char* key, const T& value)
        {
            return set(key, &value, sizeof(T));
        }

        Status get(const char* key, void* out_value, size_t length)
        {
            size_t l = length;
            return get(key, out_value, &l);
        }

        virtual Status get(const char* key, void* out_value, size_t* length) = 0;
        virtual Status set(const char* key, const void* value, size_t length) = 0;
        virtual Status erase(const char* key) = 0;
    };

    Status Init();

    Status Open(const char* namespace_name, Handle** out_handle);
    void Close(Handle* handle);
}
