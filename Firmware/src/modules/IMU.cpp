#include <freertos/FreeRTOS.h>
#include "modules/IMU.hpp"
#include "modules/I2C.hpp"
#include "modules/NVS.hpp"
#include "modules/Log.hpp"
#include "config.hpp"
#include "mpu6050.h"

namespace IMU
{
    static NVS::Handle* nvsHandle = nullptr;
    static mpu6050_handle_t mpu_handle;
    static mpu6050_gyro_value_t gyro_value;
    static mpu6050_accel_value_t acce_value;
    static mpu6050_temp_value_t temp_value;

    static mpu6050_gyro_value_t gyro_calibrated_mean;
    static mpu6050_gyro_value_t* gyro_calib_samples;
    static uint8_t calib_sample_index;
    static bool calibrating = false;

    static uint8_t temp_counter = 0;

    static Callback on_calib_done_cb = nullptr;

    void on_calib_samples_gathered()
    {
        mpu6050_gyro_value_t avg { 0, 0, 0 };
        for (uint8_t i = 0; i < IMU_NB_CALIB_SAMPLES; i++)
        {
            mpu6050_gyro_value_t& val = gyro_calib_samples[i];
            avg.gyro_x += val.gyro_x;
            avg.gyro_y += val.gyro_y;
            avg.gyro_z += val.gyro_z;
        }
        gyro_calibrated_mean.gyro_x = avg.gyro_x / (float)IMU_NB_CALIB_SAMPLES;
        gyro_calibrated_mean.gyro_y = avg.gyro_y / (float)IMU_NB_CALIB_SAMPLES;
        gyro_calibrated_mean.gyro_z = avg.gyro_z / (float)IMU_NB_CALIB_SAMPLES;

        // save calib data to nvs
        if (nvsHandle)
        {
            nvsHandle->set("gyro_calib", &gyro_calibrated_mean, sizeof(mpu6050_gyro_value_t));
        }

        calibrating = false;
        calib_sample_index = 0;
        free(gyro_calib_samples);

        if (on_calib_done_cb)
        {
            on_calib_done_cb();
            on_calib_done_cb = nullptr;
        }
    }

    Error Init()
    {
        if (I2C::Init() != Error::Ok) {
            return Error::Unknown;
        }

        // open NVS handle
        if (NVS::Open("imu", &nvsHandle) != Error::Ok) {
            return Error::Unknown;
        }

        // load saved calibration data
        if (nvsHandle->get("gyro_calib", &gyro_calibrated_mean, sizeof(mpu6050_gyro_value_t)) != Error::Ok) {
            Log::Add(Log::Level::Warning, "IMU: No saved gyro calibration data found in NVS");
            gyro_calibrated_mean = { 0, 0, 0 };
        }

        // create MPU6050 handle (primary I2C bus)
        mpu6050_info_t infos {
            .address = IMU_I2C_ADDR,
            .clock_speed = IMU_I2C_CLOCK
        };
        if (mpu6050_create(I2C::handle_primary, infos, &mpu_handle) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "IMU: Failed to create MPU6050 handle");
            return Error::Unknown;
        }

        // configure and wake up MPU6050
        mpu6050_config_t config {
            .accel_fs = ACCEL_FS_4G,
            .gyro_fs = GYRO_FS_500DPS
        };
        if (mpu6050_config(mpu_handle, config) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "IMU: Failed to configure MPU6050");
            return Error::Unknown;
        }
        if (mpu6050_wake_up(mpu_handle) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "IMU: Failed to wake up MPU6050");
            return Error::Unknown;
        }

        return Error::Ok;
    }

    Error Calibrate(Callback onDone)
    {
        if (calibrating)
            return Error::InvalidState;

        // NOTE : only calibrating gyro drift here
        gyro_calib_samples = (mpu6050_gyro_value_t*) malloc(IMU_NB_CALIB_SAMPLES * sizeof(mpu6050_gyro_value_t));
        calib_sample_index = 0;
        calibrating = true;
        on_calib_done_cb = onDone;
        
        return Error::Ok;
    }

    Error GetTemperature(float* outTemperature)
    {
        *outTemperature = temp_value.temp;
        return Error::Ok;
    }

    Error GetAngularVelocity(Vec3f* outAngularVelocity)
    {
        outAngularVelocity->x = gyro_value.gyro_x - gyro_calibrated_mean.gyro_x;
        outAngularVelocity->y = gyro_value.gyro_y - gyro_calibrated_mean.gyro_y;
        outAngularVelocity->z = gyro_value.gyro_z - gyro_calibrated_mean.gyro_z;
        return Error::Ok;
    }

    Error GetAngularVelocityRaw(Vec3f* outAngularVelocity)
    {
        outAngularVelocity->x = gyro_value.gyro_x;
        outAngularVelocity->y = gyro_value.gyro_y;
        outAngularVelocity->z = gyro_value.gyro_z;
        return Error::Ok;
    }

    Error GetLinearAcceleration(Vec3f* outLinearAcceleration)
    {
        outLinearAcceleration->x = acce_value.accel_x;
        outLinearAcceleration->y = acce_value.accel_y;
        outLinearAcceleration->z = acce_value.accel_z;
        return Error::Ok;
    }

    bool isCalibrating()
    {
        return calibrating;
    }

    float getCalibrationProgress()
    {
        if (!calibrating)
            return 1.0f;
        return (float)calib_sample_index / (float)IMU_NB_CALIB_SAMPLES;
    }
    
    void _update_task(void* pvParams)
    {
        if (mpu6050_get_gyro(mpu_handle, &gyro_value) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "IMU: Failed to get gyro data");
        }
        if (mpu6050_get_accel(mpu_handle, &acce_value) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "IMU: Failed to get accel data");
        }

        if (temp_counter++ == (1000 / 20)) // read temp less often (assuming 50Hz update for 1Hz temp)
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(mpu6050_get_temp(mpu_handle, &temp_value));
            temp_counter = 0;
        }
        if (calibrating)
        {
            gyro_calib_samples[calib_sample_index++] = gyro_value;
            if (calib_sample_index == IMU_NB_CALIB_SAMPLES)
                on_calib_samples_gathered();
        }
    }
}