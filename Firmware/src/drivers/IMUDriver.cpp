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
        if (initialized) return Error::None;
        
        if (Error err = I2C::Init(); err != Error::None)
        {
            return err;
        }

        mpu6050_info_t mpu_info = MPU6050_DEFAULT_INFO();

        if (esp_err_t err = mpu6050_create(I2C::handle_primary, mpu_info, &mpu_handle); err != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to create MPU6050 handle");
            return Error::HardwareFailure;
        }

        mpu6050_config_t mpu_config = MPU6050_DEFAULT_CONFIG();

        if (esp_err_t err = mpu6050_config(mpu_handle, mpu_config); err != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to configure MPU6050");
            return Error::HardwareFailure;
        }

        if (esp_err_t err = mpu6050_wake_up(mpu_handle); err != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to wake up MPU6050");
            return Error::HardwareFailure;
        }

        initialized = true;
        return Error::None;
    }

    Error Deinit()
    {
        if (esp_err_t err = mpu6050_delete(mpu_handle); err != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to delete MPU6050 handle");
            return Error::HardwareFailure;
        }
        initialized = false;
        return Error::None;
    }

    void __ISRReadData()
    {
        mpu6050_accel_value_t accel;
        mpu6050_gyro_value_t gyro;

        if (esp_err_t err = mpu6050_get_accel(mpu_handle, &accel); err != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to read accelerometer data from MPU6050");
        }

        if (esp_err_t err = mpu6050_get_gyro(mpu_handle, &gyro); err != ESP_OK)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to read gyroscope data from MPU6050");
        }

        imu_data.accel_x_g = accel.accel_x;
        imu_data.accel_y_g = accel.accel_y;
        imu_data.accel_z_g = accel.accel_z;
        imu_data.gyro_x_ds = gyro.gyro_x;
        imu_data.gyro_y_ds = gyro.gyro_y;
        imu_data.gyro_z_ds = gyro.gyro_z;
    }

    IMUData& GetData()
    {
        return imu_data;
    }
}
