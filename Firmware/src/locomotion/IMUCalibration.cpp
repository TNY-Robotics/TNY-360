#include "locomotion/IMU.hpp"
#include "common/Log.hpp"
#include "common/analysis/RunningStats.hpp"

Status IMU::calibration_sequence()
{
    LOG_SCOPE(TAG, "IMU::CalibrationSequence");
    LOG_INFO(TAG, "Starting calibration (please keep the robot stationary)...");

    constexpr int NB_SAMPLES = 100;
    constexpr int SAMPLE_DELAY_MS = 20;
    constexpr int MAX_ATTEMPTS = 5;

    // Thresholds for stability checks
    constexpr float MAX_GYRO_STD_DEV  = 0.010f; // rad/s
    constexpr float MAX_ACCEL_STD_DEV = 0.100f; // m/s2
    constexpr float MIN_ACCEL_NORM = 9.0f;
    constexpr float MAX_ACCEL_NORM = 12.0f;

    for (int attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt)
    {
        LOG_DEBUG(TAG, "Calibration attempt %d/%d...", attempt, MAX_ATTEMPTS);

        RunningStats gx, gy, gz;
        RunningStats ax, ay, az;

        for (int i = 0; i < NB_SAMPLES; i++)
        {
            Vec3f accel, gyro;
            if (Status err = provide_data(accel, gyro); err != Status::Ok)
            {
                calibrationState = CalibrationState::ERROR;
                return err;
            }

            gx.update(gyro.x);  gy.update(gyro.y);  gz.update(gyro.z);
            ax.update(accel.x); ay.update(accel.y); az.update(accel.z);

            // Update progress for UI feedback
            calibrationProgress = static_cast<float>(i) / NB_SAMPLES;
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_DELAY_MS));
        }

        {
            LOG_SCOPE(TAG, "Stats");
            {
                LOG_SCOPE(TAG, "Gyroscope");
                LOG_DEBUG(TAG, "X: min=%.3f, max=%.3f, mean=%.3f, std_dev=%.3f", gx.min(), gx.max(), gx.mean(), gx.std_dev());
                LOG_DEBUG(TAG, "Y: min=%.3f, max=%.3f, mean=%.3f, std_dev=%.3f", gy.min(), gy.max(), gy.mean(), gy.std_dev());
                LOG_DEBUG(TAG, "Z: min=%.3f, max=%.3f, mean=%.3f, std_dev=%.3f", gz.min(), gz.max(), gz.mean(), gz.std_dev());
            }
            {
                LOG_SCOPE(TAG, "Accelerometer");
                LOG_DEBUG(TAG, "X: min=%.3f, max=%.3f, mean=%.3f, std_dev=%.3f", ax.min(), ax.max(), ax.mean(), ax.std_dev());
                LOG_DEBUG(TAG, "Y: min=%.3f, max=%.3f, mean=%.3f, std_dev=%.3f", ay.min(), ay.max(), ay.mean(), ay.std_dev());
                LOG_DEBUG(TAG, "Z: min=%.3f, max=%.3f, mean=%.3f, std_dev=%.3f", az.min(), az.max(), az.mean(), az.std_dev());
            }
        }

        // Check stability of the gyroscope
        bool gyro_stable = (gx.std_dev() < MAX_GYRO_STD_DEV) &&
                           (gy.std_dev() < MAX_GYRO_STD_DEV) &&
                           (gz.std_dev() < MAX_GYRO_STD_DEV);
        // Check stability of the accelerometer
        bool accel_stable = (ax.std_dev() < MAX_ACCEL_STD_DEV) &&
                            (ay.std_dev() < MAX_ACCEL_STD_DEV) &&
                            (az.std_dev() < MAX_ACCEL_STD_DEV);
        // Check gravity norm
        float norm_accel = std::sqrt(ax.mean() * ax.mean() + 
                                     ay.mean() * ay.mean() + 
                                     az.mean() * az.mean());
        bool norm_ok = (norm_accel >= MIN_ACCEL_NORM && norm_accel <= MAX_ACCEL_NORM);

        if (gyro_stable && accel_stable && norm_ok)
        {
            LOG_INFO(TAG, "Robot is stationary (accel norm: %.2f m/s²).", norm_accel);

            // Set calibration data
            calibrationData.gyro_bias = Vec3f(gx.mean(), gy.mean(), gz.mean());
            calibrationData.accel_scale = IMU::GRAVITY / norm_accel;

            calibrationProgress = 1.0f;
            calibrationState = CalibrationState::CALIBRATED;
            return Status::Ok;
        }

        LOG_WARNING(TAG, "Detected movement (std_dev gyro=[%.3f, %.3f, %.3f], accel=[%.3f, %.3f, %.3f], norm=%.2f). Trying again...", gx.std_dev(), gy.std_dev(), gz.std_dev(), ax.std_dev(), ay.std_dev(), az.std_dev(), norm_accel);
    }

    LOG_ERROR(TAG, "Calibration Failed, robot is not stationary.");
    calibrationState = CalibrationState::ERROR;
    return Status::Failure;
}