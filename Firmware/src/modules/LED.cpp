#include "modules/LED.hpp"

namespace LED
{
    Error Init()
    {
        // setup led control using RMT
        return Error::Ok;
    }

    Error SetColor(Id id, Color color)
    {
        return Error::Ok; // TODO
    }

    Error SetColor(Id id, Color color, float duration_s)
    {
        return Error::Ok; // TODO
    }

    Error SetColors(const Color colors[])
    {
        return Error::Ok; // TODO
    }

    Error SetColors(const Color colors[], const float durations_s[])
    {
        return Error::Ok; // TODO
    }

    Error SetColors(const Id ids[], const Color colors[], uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error SetColors(const Id ids[], const Color colors[], const float durations_s[], uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error GetColor(Id id, Color* outColor)
    {
        return Error::Ok; // TODO
    }

    Error GetColors(Color* outColors)
    {
        return Error::Ok; // TODO
    }

    Error GetColors(const Id ids[], Color* outColors, uint8_t count)
    {
        return Error::Ok; // TODO
    }
}