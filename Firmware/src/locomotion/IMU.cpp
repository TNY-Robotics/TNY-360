#include "locomotion/IMU.hpp"
#include "drivers/IMUDriver.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

IMU::IMU()
: downVector(0.f, 0.f, -1.f)
{
    
}

Error IMU::init()
{
    LOG_SCOPE(TAG, "IMU::init");
    
    if (Error err = IMUDriver::Init(); err != Error::None)
    {
        ErrorHandle(ErrorStruct::IMUInitFailed);
        return err;
    }
    return Error::None;
}

Error IMU::deinit()
{
    if (Error err = IMUDriver::Deinit(); err != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error IMU::estimateState(float dt)
{
    // Get the data
    IMUDriver::IMUData& data = IMUDriver::GetData();

    // Estimate down vector (only using complementary filter for now, might change for madgwicks later)

    // first rotate downvector by gyro rates
    Vec3f rotated_downVector = downVector;
    Quatf q_gyro = Quatf::FromEulerAngles(Vec3f(
        DEG_TO_RAD(-data.gyro_x_ds) * dt,
        DEG_TO_RAD(-data.gyro_y_ds) * dt,
        DEG_TO_RAD(-data.gyro_z_ds) * dt
    )); // FIXME : Doing it like this tends to create really small values (numerical instability).
        // Investigate to see if it causes problems (visualization shows really big delay and slow reaction time)
    rotated_downVector = q_gyro.rotate(rotated_downVector);

    // fuse with accelerometer
    float alpha = 0.95f; // complementary filter coefficient
    Vec3f accel_vector = Vec3f(data.accel_x_g, data.accel_y_g, data.accel_z_g).normalized();
    downVector = (rotated_downVector * alpha + accel_vector * (1.0f - alpha)).normalized();

    // update orientation
    orientation.x = atan2f(downVector.y, downVector.z);
    orientation.y = atan2f(-downVector.x, sqrtf(downVector.y * downVector.y + downVector.z * downVector.z));
    orientation.z = 0.0f;

    return Error::None;
}