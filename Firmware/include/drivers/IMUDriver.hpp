#pragma once
#include "common/utils.hpp"

namespace IMUDriver
{
    constexpr const char* TAG = "IMUDriver";

    struct IMUData
    {
        float accel_x_g;
        float accel_y_g;
        float accel_z_g;
        float gyro_x_ds;
        float gyro_y_ds;
        float gyro_z_ds;
    };

    /**
    * @brief Initializes the IMU driver.
    * @return Error code indicating success or failure.
    */
    Status Init();

    /**
    * @brief Deinitializes the IMU driver.
    * @return Error code indicating success or failure.
    */
    Status Deinit();

    /**
     * @brief Internal function to read data from the MPU6050.
     * @note YOU SHOULD NOT CALL THIS FUNCTION DIRECTLY.
     * @return void.
     */
    Status ReadData();

    /**
     * @brief Get the last read values from the MPU6050.
     * @return most recent IMUData object.
     */
    IMUData& GetData();
}
