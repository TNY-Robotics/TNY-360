#include "modules/Power.hpp"

namespace Power
{
    Error Init()
    {
        return Error::Ok;
    }

    Error GetBatteryLevel(float* outLevel)
    {
        *outLevel = 100.0f;
        return Error::Ok; // TODO
    }

    Error GetVoltage(float* outVoltage)
    {
        *outVoltage = 12.1f;
        return Error::Ok; // TODO
    }

    Error GetCurrent(float* outCurrent)
    {
        *outCurrent = 0.5f;
        return Error::Ok; // TODO
    }

    Error IsCharging(bool* outIsCharging)
    {
        *outIsCharging = false;
        return Error::Ok; // TODO
    }
}