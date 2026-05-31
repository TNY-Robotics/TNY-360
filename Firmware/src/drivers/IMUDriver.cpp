#include "drivers/IMUDriver.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "mpu6050.h"

namespace IMUDriver
{
    bool initialized = false;
    static mpu6050_handle_t mpu_handle;
    static IMUData imu_data;

    Error Init()
    {
        LOG_SCOPE(TAG, "IMUDriver::Init");

        if (initialized) return Error::None;
        
        if (Error err = I2C::Init(); err != Error::None)
        {
            return err;
        }

        mpu6050_info_t mpu_info = {
            .address = IMU_I2C_ADDR,
            .clock_speed = IMU_I2C_CLOCK
        };

        if (esp_err_t err = mpu6050_create(I2C::handle_primary, mpu_info, &mpu_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to create MPU6050 handle with error: 0x%0x", err);
            ErrorHandle(ErrorStruct::IMUInitFailed);
            return Error::HardwareFailure;
        }
        
        // Note : Resetting it so it's in a known state
        if (esp_err_t err = mpu6050_reset(mpu_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to reset MPU6050 with error: 0x%0x", err);
            ErrorHandle(ErrorStruct::IMUInitFailed);
            return Error::HardwareFailure;
        }

        mpu6050_config_t mpu_config = MPU6050_DEFAULT_CONFIG();
        mpu_config.wake_auto = false; // We'll do that manually after configuration

        if (esp_err_t err = mpu6050_config(&mpu_handle, mpu_config); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to configure MPU6050 with error: 0x%0x", err);
            ErrorHandle(ErrorStruct::IMUInitFailed);
            return Error::HardwareFailure;
        }

        if (esp_err_t err = mpu6050_wake_up(mpu_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to wake up MPU6050 with error: 0x%0x", err);
            ErrorHandle(ErrorStruct::IMUInitFailed);
            return Error::HardwareFailure;
        }

        initialized = true;
        return Error::None;
    }

    Error Deinit()
    {
        if (esp_err_t err = mpu6050_delete(mpu_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to delete MPU6050 handle with error: 0x%0x", err);
            return Error::HardwareFailure;
        }
        initialized = false;
        return Error::None;
    }

    Error ReadData()
    {
        mpu6050_accel_value_t accel;
        mpu6050_gyro_value_t gyro;

        if (esp_err_t err = mpu6050_get_all(mpu_handle, &accel, &gyro, nullptr); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to read sensor data from MPU6050 with error: 0x%0x", err);
            return Error::HardwareFailure;
        }

        imu_data.accel_x_g = accel.accel_x;
        imu_data.accel_y_g = accel.accel_y;
        imu_data.accel_z_g = accel.accel_z;
        imu_data.gyro_x_ds = gyro.gyro_x;
        imu_data.gyro_y_ds = gyro.gyro_y;
        imu_data.gyro_z_ds = gyro.gyro_z;

        return Error::None;
    }

    IMUData& GetData()
    {
        return imu_data;
    }
}
