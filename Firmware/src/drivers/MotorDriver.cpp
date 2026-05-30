#include <freertos/FreeRTOS.h>
#include "drivers/MotorDriver.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "pca9685.h"
#include <cmath>
#include <memory.h>

namespace MotorDriver
{
    constexpr const char* TAG = "MotorDriver";

    bool initialized = false;
    pca9685_handle_t pca_handle;
    uint16_t pwm_buffer[CHANNEL_COUNT] = {0};

    Error Init()
    {
        LOG_SCOPE(TAG, "MotorDriver::Init");

        if (initialized) return Error::None;

        if (Error err = I2C::Init(); err != Error::None)
        {
            LOG_ERROR(TAG, "Failed to initialize I2C for motor driver");
            ErrorHandle(ErrorStruct::DriverInitFailed);
            return err;
        }

        // Create PCA9685 handle
        {
            pca9685_info_t pca_info = {
                .address = MOTOR_DRIVER_I2C_ADDR,
                .clock_speed = 1'000'000, // Fast-mode+ (1Mhz)
            };
            esp_err_t err = pca9685_create(I2C::handle_primary, pca_info, &pca_handle);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to create PCA9685 handle with error : 0x%0x", err);
                ErrorHandle(ErrorStruct::DriverInitFailed);
                return Error::HardwareFailure;
            }
        }

        // Reset PCA9685 to ensure it's in a known state
        {
            esp_err_t err = pca9685_reset(pca_handle);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to reset PCA9685 with error : 0x%0x", err);
                ErrorHandle(ErrorStruct::DriverInitFailed);
                return Error::HardwareFailure;
            }
        }

        // Configure PCA9685
        {
            pca9685_config_t pca_config = {
                .frequency_hz = MOTOR_DRIVER_PWM_FREQUENCY_HZ
            };
            esp_err_t err = pca9685_config(pca_handle, pca_config);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to configure PCA9685 with error : 0x%0x", err);
                ErrorHandle(ErrorStruct::DriverInitFailed);
                return Error::HardwareFailure;
            }
        }

        // Wake up the motor driver to be sure it's ready
        {
            esp_err_t err = pca9685_wake_up(pca_handle);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to wake up PCA9685 with error : 0x%0x", err);
                ErrorHandle(ErrorStruct::DriverInitFailed);
                return Error::HardwareFailure;
            }
        }

        initialized = true;
        return Error::None;
    }

    Error Deinit()
    {
        if (!initialized) return Error::None;

        // Disable all motors before deinitializing
        DisableAllMotors();

        pca9685_delete(pca_handle);

        initialized = false;
        return Error::None;
    }

    Error SetPWM(Channel id, Value pwm)
    {
        if (!initialized)
        {
            LOG_ERROR(TAG, "MotorDriver not initialized");
            return Error::InvalidState;
        }

        if (id >= CHANNEL_COUNT)
        {
            LOG_ERROR(TAG, "Invalid motor ID: %d", id);
            return Error::InvalidParameters;
        }

        if (pwm > 4096)
        {
            LOG_ERROR(TAG, "Invalid PWM value: %d", pwm);
            return Error::InvalidParameters;
        }

        pwm_buffer[id] = pwm;
        return Error::None;
    }

    Error GetPWM(Channel id, Value &pwm)
    {
        if (!initialized)
        {
            LOG_ERROR(TAG, "MotorDriver not initialized");
            return Error::InvalidState;
        }

        if (id >= CHANNEL_COUNT)
        {
            LOG_ERROR(TAG, "Invalid motor ID: %d", id);
            return Error::InvalidParameters;
        }

        pwm = pwm_buffer[id];
        return Error::None;
    }

    Error DisableAllMotors()
    {
        if (!initialized)
        {
            LOG_ERROR(TAG, "MotorDriver not initialized");
            return Error::InvalidState;
        }

        memset(pwm_buffer, 0, sizeof(pwm_buffer));

        // this is generally called when something bad happened, so we send the values immediately
        SendData();

        return Error::None;
    }
    
    Error SendData()
    {
        if (esp_err_t err = pca9685_set_pwms(pca_handle, pwm_buffer); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to set PWM values with error : 0x%0x", err);
            return Error::HardwareFailure;
        }
        return Error::None;
    }
}