#include <freertos/FreeRTOS.h>
#include "modules/Motor.hpp"
#include "modules/AnalogScanner.hpp"
#include "config.hpp"
#include "pca9685.h"

namespace Motor
{
    constexpr float UPDATE_INTERVAL_S = MOTORS_UPDT_INT_MS / 1000.f;

    bool initialized = false;

    float freq_min[NB_MOTORS] = {
        190, 100, 120, 0,
        150, 100, 130, 0,
        250, 100, 105, 0,
        240, 105, 130, 0};
    float freq_range[NB_MOTORS] = {
        230, 410, 350, 1,
        220, 415, 345, 1,
        225, 420, 350, 1,
        215, 420, 350, 1};
    float angle_min[NB_MOTORS] = {
        45, 90, 135, 0,
        -45, -90, 0, 0,
        -45, 90, 135, 0,
        45, -90, 0, 0};
    float angle_range[NB_MOTORS] = {
        -90, -180, -135, 1,
        90, 180, 135, 1,
        90, -180, -135, 1,
        -90, 180, 135, 1};

    float last_motor_freq[NB_MOTORS] = { // for smooth transition with duration
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    float target_motor_freq[NB_MOTORS] = { // all deactivated by default
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    float motor_adder[NB_MOTORS] = { // value to add to last_motor_freq to match movement duration
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    uint8_t motor_force_countdown[NB_MOTORS] = { // force divider for each motor (1 is full force, 5 is force 1/5 of the time)
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    inline AnalogScanner::Id castId(Id id)
    {
        return static_cast<AnalogScanner::Id>(static_cast<uint8_t>(id) + static_cast<uint8_t>(AnalogScanner::Id::Shoulder_FL));
    }

    Error Init()
    {
        return Error::Ok;
    }

    Error SetAngle(Id id, float angle)
    {
        return Error::Ok; // TODO
    }

    Error SetAngle(Id id, float angle, float duration_s)
    {
        return Error::Ok; // TODO
    }

    Error SetAngles(const float* angles)
    {
        return Error::Ok; // TODO
    }

    Error SetAngles(const float* angles, const float* durations_s)
    {
        return Error::Ok; // TODO
    }

    Error SetAngles(const Id* ids, const float* angles, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error SetAngles(const Id* ids, const float* angles, const float* durations_s, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error SetForce(Id id, ForceLevel force)
    {
        return Error::Ok; // TODO
    }

    Error SetForces(const ForceLevel* forces)
    {
        return Error::Ok; // TODO
    }

    Error SetForces(const Id* ids, const ForceLevel* forces, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error GetForce(Id id, ForceLevel* outForce)
    {
        return Error::Ok; // TODO
    }

    Error GetForces(ForceLevel* outForces)
    {
        return Error::Ok; // TODO
    }

    Error GetForces(const Id* ids, ForceLevel* outForces, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error GetTargetAngle(Id id, float* outAngle)
    {
        return Error::Ok; // TODO
    }

    Error GetTargetAngles(float* outAngles)
    {
        return Error::Ok; // TODO
    }

    Error GetTargetAngles(const Id* ids, float* outAngles, uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error GetCurrentAngle(Id id, float* outAngle)
    {
        return AnalogScanner::GetValue(castId(id), outAngle);
    }

    Error GetCurrentAngles(float* outAngles)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            if (AnalogScanner::GetValue(castId(static_cast<Id>(i)), &outAngles[i]) != Error::Ok)
                return Error::Unknown;
        }
        return Error::Ok;
    }

    Error GetCurrentAngles(const Id* ids, float* outAngles, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            if (AnalogScanner::GetValue(castId(ids[i]), &outAngles[i]) != Error::Ok)
                return Error::Unknown;
        }
        return Error::Ok;
    }
}