#include "locomotion/IMUController.hpp"
#include "drivers/IMUDriver.hpp"
#include "common/config.hpp"
#include "common/Log.hpp"

IMUController::IMUController()
: downVector(0.f, 0.f, -1.f)
{
    
}

Error IMUController::init()
{
    if (Error err = IMUDriver::Init(); err != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error IMUController::deinit()
{
    if (Error err = IMUDriver::Deinit(); err != Error::None)
    {
        return err;
    }
    return Error::None;
}

Error IMUController::update()
{
    // Get the data
    IMUDriver::IMUData& data = IMUDriver::GetData();

    // Estimate down vector (only using complementary filter for now, might change for madgwicks later)

    // first rotate downvector by gyro rates
    Vec3f rotated_downVector = downVector;
    Quatf q_gyro = Quatf::FromEulerAngles(Vec3f(
        DEG_TO_RAD(-data.gyro_x_ds) * CONTROL_LOOP_DT_S,
        DEG_TO_RAD(-data.gyro_y_ds) * CONTROL_LOOP_DT_S,
        DEG_TO_RAD(-data.gyro_z_ds) * CONTROL_LOOP_DT_S
    )); // FIXME : Doing it like this tends to create really small values (numerical instability). Investigate to see if it causes problems (visualization shows really big delay and slow reaction time)
    rotated_downVector = q_gyro.rotate(rotated_downVector);

    // fuse with accelerometer
    float alpha = 0.95f; // complementary filter coefficient
    Vec3f accel_vector = Vec3f(data.accel_x_g, data.accel_y_g, data.accel_z_g).normalized();
    downVector = (rotated_downVector * alpha + accel_vector * (1.0f - alpha)).normalized();

    // update orientation quaternion
    float roll = atan2f(downVector.y, downVector.z);
    float pitch = atan2f(-downVector.x, sqrtf(downVector.y * downVector.y + downVector.z * downVector.z));
    
    orientation = Quatf::FromEulerAngles(Vec3f(-roll, 0.0f, pitch)); // FIXME : Dirty fix to make the THREEJS visualization correct, but the robot is z-up right-handed, not y-up left-handed !! Conversion should be done on the website instead !
    orientation.normalize();

    return Error::None;
}