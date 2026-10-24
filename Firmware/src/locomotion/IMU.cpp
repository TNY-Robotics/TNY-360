#include "locomotion/IMU.hpp"
#include "common/geometry.hpp"
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
    // Get the data from underlying IMU implementation
    if (Status err = provide_data(acceleration, angularVelocity); err != Status::Ok)
    {
        return err;
    }

    // Apply gyro bias correction
    angularVelocity -= gyro_bias;

    float gx = angularVelocity.x; 
    float gy = angularVelocity.y;
    float gz = angularVelocity.z;

    // Get quaternion derivative from gyroscope data
    float qDot_w = 0.5f * (-q.x * gx - q.y * gy - q.z * gz);
    float qDot_x = 0.5f * ( q.w * gx + q.y * gz - q.z * gy);
    float qDot_y = 0.5f * ( q.w * gy - q.x * gz + q.z * gx);
    float qDot_z = 0.5f * ( q.w * gz + q.x * gy - q.y * gx);

    // If there's valid accelerometer data, use it to correct
    if (acceleration.lengthSq() > 0.0f)
    {
        Vec3f a = acceleration.normalized();

        // Calculate this once here to avoid repeated calculations after
        float _2qw = 2.0f * q.w, _2qx = 2.0f * q.x, _2qy = 2.0f * q.y, _2qz = 2.0f * q.z;
        float _4qw = 4.0f * q.w, _4qx = 4.0f * q.x, _4qy = 4.0f * q.y, _8qx = 8.0f * q.x, _8qy = 8.0f * q.y;
        float qwqw = q.w * q.w, qxqx = q.x * q.x, qyqy = q.y * q.y, qzqz = q.z * q.z;

        // Gradient descent algorithm (correction step)
        float s_w = _4qw * qyqy + _2qy * a.x + _4qw * qxqx - _2qx * a.y;
        float s_x = _4qx * qzqz - _2qz * a.x + 4.0f * qwqw * q.x - _2qw * a.y - _4qx + _8qx * qxqx + _8qx * qyqy + _4qx * a.z;
        float s_y = 4.0f * qwqw * q.y + _2qw * a.x + _4qy * qzqz - _2qz * a.y - _4qy + _8qy * qxqx + _8qy * qyqy + _4qy * a.z;
        float s_z = 4.0f * qxqx * q.z - _2qx * a.x + 4.0f * qyqy * q.z - _2qy * a.y;

        // Normalize the gradient
        float recipNorm = 1.0f / std::sqrt(s_w * s_w + s_x * s_x + s_y * s_y + s_z * s_z);
        s_w *= recipNorm; s_x *= recipNorm; s_y *= recipNorm; s_z *= recipNorm;

        // Apply gain
        qDot_w -= beta * s_w;
        qDot_x -= beta * s_x;
        qDot_y -= beta * s_y;
        qDot_z -= beta * s_z;
    }

    // Integrate to get new quaternion
    q.w += qDot_w * dt;
    q.x += qDot_x * dt;
    q.y += qDot_y * dt;
    q.z += qDot_z * dt;

    // Normalize just to be safe
    q.normalize();

    // Export to orientation vector
    orientation = q;

    downVector = q.rotate(Vec3f(0.f, 0.f, -1.f));

    return Status::Ok;
}