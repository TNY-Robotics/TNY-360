#include "drivers/IMUDriver.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "drivers/IMUDriver.Error.hpp"
#include "mpu6050.h"

namespace IMUDriver
{
    bool initialized = false;
    static mpu6050_handle_t mpu_handle;
    static IMUData imu_data;

    Status Init()
    {
        LOG_SCOPE(TAG, "IMUDriver::Init");

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

    Status Deinit()
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

    Status ReadData()
    {
        mpu6050_accel_value_t accel;
        mpu6050_gyro_value_t gyro;

        if (esp_err_t err = mpu6050_get_all(mpu_handle, &accel, &gyro, nullptr); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to read sensor data from MPU6050 with error: 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventReadDataFailed(err));
            return Status::Failure;
        }

        imu_data.accel_x_g = accel.accel_x;
        imu_data.accel_y_g = accel.accel_y;
        imu_data.accel_z_g = accel.accel_z;
        imu_data.gyro_x_ds = gyro.gyro_x;
        imu_data.gyro_y_ds = gyro.gyro_y;
        imu_data.gyro_z_ds = gyro.gyro_z;

        return Status::Ok;
    }

    IMUData& GetData()
    {
        return imu_data;
    }
}
