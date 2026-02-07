#pragma once
#include "common/utils.hpp"

namespace IMUDriver
{
    constexpr const char* TAG = "IMUDriver";

    typedef struct IMUData
    {
        float accel_x_g;
        float accel_y_g;
        float accel_z_g;
        float gyro_x_ds;
        float gyro_y_ds;
        float gyro_z_ds;
    } IMUData;

    /**
    * @brief Initializes the IMU driver.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Deinitializes the IMU driver.
    * @return Error code indicating success or failure.
    */
    Error Deinit();

    void __ISRReadData();

    IMUData& GetData();
}
