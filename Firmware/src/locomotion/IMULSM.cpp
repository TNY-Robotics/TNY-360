#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/geometry.hpp"
#include "locomotion/IMU.Error.hpp"
#include "locomotion/IMULSM.hpp"
#include "lsm6ds3.h"

static lsm6ds3_handle_t lsm_handle;

Status IMULSM::init()
{
    LOG_SCOPE(TAG, "IMULSM::Init");

    if (initialized) return Status::Ok;
    
    if (Status err = I2C::Init(); err != Status::Ok)
    {
        Error::RegisterErrorEvent(ErrorEventI2CInitFailed());
        return err;
    }

    lsm6ds3_info_t lsm_info = LSM6DS3_DEFAULT_INFO();
    if (esp_err_t err = lsm6ds3_create(I2C::handle_primary, lsm_info, &lsm_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to create LSM6DS3 handle with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventCreateFailed(err));
        return Status::Failure;
    }

    lsm6ds3_config_t lsm_config = LSM6DS3_DEFAULT_CONFIG();
    if (esp_err_t err = lsm6ds3_config(&lsm_handle, lsm_config); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to configure LSM6DS3 with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventConfigFailed(err));
        return Status::Failure;
    }

    // TODO : Maybe wake up or something ?

    initialized = true;
    return Status::Ok;
}

Status IMULSM::deinit()
{
    if (!initialized) return Status::Ok;

    if (esp_err_t err = lsm6ds3_delete(lsm_handle); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to delete LSM6DS3 handle with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventDeleteFailed(err));
        return Status::Failure;
    }

    initialized = false;
    return Status::Ok;
}

Status IMULSM::provide_data(Vec3f &accel, Vec3f &angular_vel)
{
    if (!initialized) return Status::InvalidState;

    lsm6ds3_accel_value_t raw_accel;
    lsm6ds3_gyro_value_t raw_gyro;

    if (esp_err_t err = lsm6ds3_get_all(lsm_handle, &raw_accel, &raw_gyro, nullptr); err != ESP_OK)
    {
        LOG_ERROR(TAG, "Failed to read sensor data from LSM6DS3 with error: 0x%0x", err);
        Error::RegisterErrorEvent(ErrorEventReadDataFailed(err));
        return Status::Failure;
    }

    // the IMU isnt aligned with the robot frame, so we need to match the axes
    accel.x = raw_accel.accel_y * IMU::GRAVITY;
    accel.y = raw_accel.accel_x * IMU::GRAVITY;
    accel.z = -raw_accel.accel_z * IMU::GRAVITY;
    angular_vel.x = DEG_TO_RAD(raw_gyro.gyro_y);
    angular_vel.y = DEG_TO_RAD(raw_gyro.gyro_x);
    angular_vel.z = DEG_TO_RAD(-raw_gyro.gyro_z);

    return Status::Ok;
}
