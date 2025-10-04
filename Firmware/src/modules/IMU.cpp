#include "modules/IMU.hpp"
#include "modules/I2C.hpp"
#include "modules/NVS.hpp"

namespace IMU
{
    NVS::Handle* nvsHandle = nullptr;

    static Error Init()
    {
        if (I2C::Init() != Error::Ok) {
            return Error::Unknown;
        }

        if (NVS::Open("imu", &nvsHandle) != Error::Ok) {
            return Error::Unknown;
        }

        // setup using I2C::handle for communication with the IMU sensor
        // + setup reading task, data processing, etc.

        return Error::Ok;
    }

    static Error Calibrate(Callback onDone)
    {
        if (onDone) {
            onDone();
        }
        return Error::Ok; // TODO
    }

    static Error GetTemperature(float* outTemperature)
    {
        *outTemperature = 22.0f; // Simulate room temperature
        return Error::Ok; // TODO
    }

    static Error GetAngularVelocity(Vec3f* outAngularVelocity)
    {
        *outAngularVelocity = Vec3f(0.0f, 0.0f, 0.0f); // Simulate stationary
        return Error::Ok; // TODO
    }

    static Error GetLinearAcceleration(Vec3f* outLinearAcceleration)
    {
        *outLinearAcceleration = Vec3f(0.0f, 0.0f, -9.81f); // Simulate stationary with gravity
        return Error::Ok; // TODO
    }
}