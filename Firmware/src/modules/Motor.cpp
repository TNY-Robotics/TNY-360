#include "modules/Motor.hpp"
#include "modules/AnalogScanner.hpp"

namespace Motor
{
    inline AnalogScanner::Id castId(Id id)
    {
        return static_cast<AnalogScanner::Id>(static_cast<uint8_t>(id) + static_cast<uint8_t>(AnalogScanner::Id::Shoulder_FL));
    }

    static Error Init()
    {
        return Error::Ok;
    }

    static Error SetAngle(Id id, float angle)
    {
        return Error::Ok; // TODO
    }

    static Error SetAngle(Id id, float angle, float duration_s)
    {
        return Error::Ok; // TODO
    }

    static Error SetAngles(const float* angles)
    {
        return Error::Ok; // TODO
    }

    static Error SetAngles(const float* angles, const float* durations_s)
    {
        return Error::Ok; // TODO
    }

    static Error SetAngles(const Id* ids, const float* angles, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error SetAngles(const Id* ids, const float* angles, const float* durations_s, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error SetForce(Id id, ForceLevel force)
    {
        return Error::Ok; // TODO
    }

    static Error SetForces(const ForceLevel* forces)
    {
        return Error::Ok; // TODO
    }

    static Error SetForces(const Id* ids, const ForceLevel* forces, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error GetForce(Id id, ForceLevel* outForce)
    {
        return Error::Ok; // TODO
    }

    static Error GetForces(ForceLevel* outForces)
    {
        return Error::Ok; // TODO
    }

    static Error GetForces(const Id* ids, ForceLevel* outForces, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error GetTargetAngle(Id id, float* outAngle)
    {
        return Error::Ok; // TODO
    }

    static Error GetTargetAngles(float* outAngles)
    {
        return Error::Ok; // TODO
    }

    static Error GetTargetAngles(const Id* ids, float* outAngles, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error GetCurrentAngle(Id id, float* outAngle)
    {
        return AnalogScanner::GetValue(castId(id), outAngle);
    }

    static Error GetCurrentAngles(float* outAngles)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            if (AnalogScanner::GetValue(castId(static_cast<Id>(i)), &outAngles[i]) != Error::Ok)
                return Error::Unknown;
        }
        return Error::Ok;
    }

    static Error GetCurrentAngles(const Id* ids, float* outAngles, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            if (AnalogScanner::GetValue(castId(ids[i]), &outAngles[i]) != Error::Ok)
                return Error::Unknown;
        }
        return Error::Ok;
    }
}