#include "locomotion/IMU.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"
#include "settings/Settings.hpp"
#include "locomotion/IMUMPU.hpp"
#include "locomotion/IMULSM.hpp"

IMU* IMU::Create()
{
    // Determine which IMU to use based on settings
    IMUType imu_type = Settings::GetConfig().imu.type;
    
    if (imu_type == IMUType::IMU_MPU6050)
    {
        return new IMUMPU();
    }
    else if (imu_type == IMUType::IMU_LSM6DS3)
    {
        return new IMULSM();
    }
    return nullptr;
}

Status IMU::estimateState(float dt)
{
    // Get the data
    
    // Underlying IMU driver call to get the data from the IMU
    if (Status err = provide_data(acceleration, angularVelocity); err != Status::Ok)
    {
        return err;
    }
    // TODO : When calibration is implemented, apply it here

    // Estimate down vector (only using complementary filter for now, might change for madgwicks later)

    // first rotate downvector by gyro rates
    Vec3f rotated_downVector = downVector;
    Quatf q_gyro = Quatf::FromEulerAngles(Vec3f(
        angularVelocity.x * dt,
        angularVelocity.y * dt,
        angularVelocity.z * dt
    ));
    rotated_downVector = q_gyro.rotate(rotated_downVector);

    // fuse with accelerometer
    float alpha = 0.95f; // complementary filter coefficient
    Vec3f accel_vector = Vec3f(acceleration.x, acceleration.y, acceleration.z).normalized();
    downVector = (rotated_downVector * alpha + accel_vector * (1.0f - alpha)).normalized();

    // update orientation
    orientation.x = atan2f(downVector.y, downVector.z);
    orientation.y = atan2f(-downVector.x, sqrtf(downVector.y * downVector.y + downVector.z * downVector.z));
    orientation.z = 0.0f;

    return Status::Ok;
}