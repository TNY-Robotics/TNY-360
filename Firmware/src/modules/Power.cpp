#include "modules/Power.hpp"

namespace Power
{
    static Error Init()
    {
        return Error::Ok;
    }

    static Error GetBatteryLevel(float* outLevel)
    {
        *outLevel = 100.0f;
        return Error::Ok; // TODO
    }

    static Error GetVoltage(float* outVoltage)
    {
        *outVoltage = 12.1f;
        return Error::Ok; // TODO
    }

    static Error GetCurrent(float* outCurrent)
    {
        *outCurrent = 0.5f;
        return Error::Ok; // TODO
    }

    static Error IsCharging(bool* outIsCharging)
    {
        *outIsCharging = false;
        return Error::Ok; // TODO
    }
}