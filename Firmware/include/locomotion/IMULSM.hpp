#pragma once
#include "common/utils.hpp"
#include "locomotion/IMU.hpp"

class IMULSM : public IMU
{
public:
    Status init() override;

    Status deinit() override;

    Status provide_data(Vec3f &accel, Vec3f &angular_vel) override;

private:
    bool initialized = false;
};
