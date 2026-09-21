#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/geometry.hpp"
#include "locomotion/IMU.Error.hpp"
#include "locomotion/IMULSM.hpp"

Status IMULSM::init()
{
    LOG_SCOPE(TAG, "IMULSM::Init");

    if (initialized) return Status::Ok;
    
    if (Status err = I2C::Init(); err != Status::Ok)
    {
        Error::RegisterErrorEvent(ErrorEventI2CInitFailed());
        return err;
    }

    // TODO : Implement LSM6DS3 driver initialization here

    initialized = true;
    return Status::Ok;
}

Status IMULSM::deinit()
{
    // TODO

    initialized = false;
    return Status::Ok;
}

Status IMULSM::provide_data(Vec3f &accel, Vec3f &angular_vel)
{
    // TODO

    return Status::Failure;
}
