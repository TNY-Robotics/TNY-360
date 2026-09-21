#pragma once
#include "common/utils.hpp"
#include "locomotion/IMU.hpp"
#include "mpu6050.h"

class IMUMPU : public IMU
{
public:
    Status init() override;

    Status deinit() override;

    Status provide_data(Vec3f &accel, Vec3f &angular_vel) override;

private:
    bool initialized = false;
    mpu6050_handle_t mpu_handle;
};
