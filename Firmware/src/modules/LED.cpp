#include "modules/LED.hpp"

namespace LED
{
    static Error Init()
    {
        // setup led control using RMT
        return Error::Ok;
    }

    static Error SetColor(Id id, Color color)
    {
        return Error::Ok; // TODO
    }

    static Error SetColor(Id id, Color color, float duration_s)
    {
        return Error::Ok; // TODO
    }

    static Error SetColors(const Color colors[])
    {
        return Error::Ok; // TODO
    }

    static Error SetColors(const Color colors[], const float durations_s[])
    {
        return Error::Ok; // TODO
    }

    static Error SetColors(const Id ids[], const Color colors[], uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error SetColors(const Id ids[], const Color colors[], const float durations_s[], uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error GetColor(Id id, Color* outColor)
    {
        return Error::Ok; // TODO
    }

    static Error GetColors(Color* outColors)
    {
        return Error::Ok; // TODO
    }

    static Error GetColors(const Id ids[], Color* outColors, uint8_t count)
    {
        return Error::Ok; // TODO
    }
}