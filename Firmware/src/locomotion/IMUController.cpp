#include "locomotion/IMUController.hpp"
#include "drivers/IMUDriver.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

IMUController::IMUController()
: downVector(0.f, 0.f, -1.f)
{
    
}

Status IMUController::init()
{
    LOG_SCOPE(TAG, "IMUController::init");
    
    if (Status err = IMUDriver::Init(); err != Status::Ok)
    {
        return err;
    }
    return Status::Ok;
}

Status IMUController::deinit()
{
    if (Status err = IMUDriver::Deinit(); err != Status::Ok)
    {
        return err;
    }
    return Status::Ok;
}

Status IMUController::estimateState(float dt)
{
    // Get the data
    IMUDriver::IMUData& data = IMUDriver::GetData();
    // TODO : When calibration is implemented, apply it here
    angularVelocity.x = DEG_TO_RAD(data.gyro_x_ds);
    angularVelocity.y = DEG_TO_RAD(data.gyro_y_ds);
    angularVelocity.z = DEG_TO_RAD(data.gyro_z_ds);
    acceleration.x = data.accel_x_g * GRAVITY;
    acceleration.y = data.accel_y_g * GRAVITY;
    acceleration.z = data.accel_z_g * GRAVITY;

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
    Vec3f accel_vector = Vec3f(data.accel_x_g, data.accel_y_g, data.accel_z_g).normalized();
    downVector = (rotated_downVector * alpha + accel_vector * (1.0f - alpha)).normalized();

    // update orientation
    orientation.x = atan2f(downVector.y, downVector.z);
    orientation.y = atan2f(-downVector.x, sqrtf(downVector.y * downVector.y + downVector.z * downVector.z));
    orientation.z = 0.0f;

    return Status::Ok;
}