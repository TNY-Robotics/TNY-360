#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/geometry.hpp"
#include "locomotion/IMU.Error.hpp"
#include "locomotion/IMUMPU.hpp"

Status IMUMPU::init()
{
    LOG_SCOPE(TAG, "IMUMPU::Init");

    if (initialized) return Status::Ok;
    
    if (Status err = I2C::Init(); err != Status::Ok)
    {
        Error::RegisterErrorEvent(ErrorEventI2CInitFailed());
        return err;
    }

    mpu6050_info_t mpu_info = {
        .address = IMU_I2C_ADDR,
        .clock_speed = IMU_I2C_CLOCK
    };

    if (esp_err_t err = mpu6050_create(I2C::handle_primary, mpu_info, &mpu_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to create MPU6050 handle with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventCreateFailed(err));
        return Status::Failure;
    }
    
    // Note : Resetting it so it's in a known state
    if (esp_err_t err = mpu6050_reset(mpu_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to reset MPU6050 with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventResetFailed(err));
        return Status::Failure;
    }

    mpu6050_config_t mpu_config = MPU6050_DEFAULT_CONFIG();
    mpu_config.wake_auto = false; // We'll do that manually after configuration

    if (esp_err_t err = mpu6050_config(&mpu_handle, mpu_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to configure MPU6050 with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventConfigFailed(err));
        return Status::Failure;
    }

    if (esp_err_t err = mpu6050_wake_up(mpu_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to wake up MPU6050 with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventWakeUpFailed(err));
        return Status::Failure;
    }

    initialized = true;
    return Status::Ok;
}

Status IMUMPU::deinit()
{
    if (esp_err_t err = mpu6050_delete(mpu_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to delete MPU6050 handle with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventDeleteFailed(err));
        return Status::Failure;
    }
    initialized = false;
    return Status::Ok;
}

Status IMUMPU::provide_data(Vec3f &accel, Vec3f &angular_vel)
{
    mpu6050_accel_value_t value_accel;
    mpu6050_gyro_value_t value_gyro;

    if (esp_err_t err = mpu6050_get_all(mpu_handle, &value_accel, &value_gyro, nullptr); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to read sensor data from MPU6050 with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventReadDataFailed(err));
        return Status::Failure;
    }

    accel.x = value_accel.accel_x * IMU::GRAVITY;
    accel.y = value_accel.accel_y * IMU::GRAVITY;
    accel.z = value_accel.accel_z * IMU::GRAVITY;

    angular_vel.x = DEG_TO_RAD(value_gyro.gyro_x);
    angular_vel.y = DEG_TO_RAD(value_gyro.gyro_y);
    angular_vel.z = DEG_TO_RAD(value_gyro.gyro_z);

    return Status::Ok;
}
