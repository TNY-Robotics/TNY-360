#include "modules/Pressure.hpp"
#include "modules/AnalogScanner.hpp"

namespace Pressure
{
    inline AnalogScanner::Id castId(Id id)
    {
        return static_cast<AnalogScanner::Id>(static_cast<uint8_t>(id) + static_cast<uint8_t>(AnalogScanner::Id::Pressure_FL));
    }

    static Error Init()
    {
        if (AnalogScanner::Init() != Error::Ok)
            return Error::Unknown;
        return Error::Ok;
    }

    static Error Calibrate(Callback onDone)
    {
        // todo : calibration sequence, then call onDone()

        return Error::Ok; // TODO
    }

    static Error GetPressure(Id id, float* outPressure)
    {
        return AnalogScanner::GetValue(castId(id), outPressure);
    }

    static Error GetPressures(float* outPressures)
    {
        if (!outPressures)
            return Error::InvalidParameters;

        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); ++i)
        {
            if (GetPressure(static_cast<Id>(i), &outPressures[i]) != Error::Ok)
                return Error::Unknown;
        }

        return Error::Ok;
    }

    static Error GetPressures(const Id* ids, float* outPressures, uint8_t count)
    {
        if (!ids || !outPressures || count == 0)
            return Error::InvalidParameters;

        for (uint8_t i = 0; i < count; ++i)
        {
            if (GetPressure(ids[i], &outPressures[i]) != Error::Ok)
                return Error::Unknown;
        }

        return Error::Ok;
    }
}