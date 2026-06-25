#include <freertos/FreeRTOS.h>
#include "drivers/MotorDriver.hpp"
#include "common/I2C.hpp"
#include "common/Log.hpp"
#include "common/config.hpp"
#include "drivers/MotorDriver.Error.hpp"
#include "pca9685.h"
#include <cmath>
#include <memory.h>

namespace MotorDriver
{
    constexpr const char* TAG = "MotorDriver";

    bool initialized = false;
    pca9685_handle_t pca_handle;
    uint16_t pwm_buffer[CHANNEL_COUNT] = {0};

    Status Init()
    {
        LOG_SCOPE(TAG, "MotorDriver::Init");

        if (initialized) return Status::Ok;

        if (Status err = I2C::Init(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to initialize I2C for motor driver");
            Error::RegisterErrorEvent(ErrorEventI2CInitFailed());
            return err;
        }

        // Create PCA9685 handle
        {
            pca9685_info_t pca_info = {
                .address = MOTOR_DRIVER_I2C_ADDR,
                .clock_speed = MOTOR_DRIVER_I2C_CLOCK,
            };
            esp_err_t err = pca9685_create(I2C::handle_primary, pca_info, &pca_handle);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to create PCA9685 handle with error : 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventCreateFailed(err));
                return Status::Failure;
            }
        }

        // Reset PCA9685 to ensure it's in a known state
        {
            esp_err_t err = pca9685_reset(pca_handle);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to reset PCA9685 with error : 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventResetFailed(err));
                return Status::Failure;
            }
        }

        // Configure PCA9685
        {
            pca9685_config_t pca_config = {
                .frequency_hz = (uint16_t) MOTOR_DRIVER_PWM_FREQUENCY_HZ
            };
            esp_err_t err = pca9685_config(pca_handle, pca_config);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to configure PCA9685 with error : 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventConfigFailed(err));
                return Status::Failure;
            }
        }

        // Wake up the motor driver to be sure it's ready
        {
            esp_err_t err = pca9685_wake_up(pca_handle);
            if (err != ESP_OK)
            {
                LOG_ERROR(TAG, "Failed to wake up PCA9685 with error : 0x%0x", err);
                Error::RegisterErrorEvent(ErrorEventWakeUpFailed(err));
                return Status::Failure;
            }
        }

        initialized = true;
        return Status::Ok;
    }

    Status Deinit()
    {
        if (!initialized) return Status::Ok;

        // Disable all motors before deinitializing
        DisableAllMotors();

        if (esp_err_t err = pca9685_delete(pca_handle); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to delete PCA9685 handle with error : 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventDeleteFailed(err));
            return Status::Failure;
        }

        initialized = false;
        return Status::Ok;
    }

    Status SetDutyCycle(Channel id, Value duty_cycle)
    {
        if (!initialized)
        {
            LOG_ERROR(TAG, "MotorDriver not initialized");
            return Status::InvalidState;
        }

        if (id >= CHANNEL_COUNT)
        {
            LOG_ERROR(TAG, "Invalid motor ID: %d", id);
            return Status::InvalidParameters;
        }

        uint16_t pwm = DC_TO_PWM(duty_cycle);
        if (pwm > 4096)
        {
            LOG_ERROR(TAG, "Invalid duty cycle value: %f", duty_cycle);
            return Status::InvalidParameters;
        }

        pwm_buffer[id] = pwm;
        return Status::Ok;
    }

    Status GetDutyCycle(Channel id, Value &duty_cycle)
    {
        if (!initialized)
        {
            LOG_ERROR(TAG, "MotorDriver not initialized");
            return Status::InvalidState;
        }

        if (id >= CHANNEL_COUNT)
        {
            LOG_ERROR(TAG, "Invalid motor ID: %d", id);
            return Status::InvalidParameters;
        }

        duty_cycle = PWM_TO_DC(pwm_buffer[id]);
        return Status::Ok;
    }

    Status DisableAllMotors()
    {
        if (!initialized)
        {
            LOG_ERROR(TAG, "MotorDriver not initialized");
            return Status::InvalidState;
        }

        memset(pwm_buffer, 0, sizeof(pwm_buffer));

        // this is generally called when something bad happened, so we send the values immediately
        if (Status err = SendData(); err != Status::Ok)
        {
            LOG_ERROR(TAG, "Failed to send PWM values to PCA9685");
            return err;
        }

        return Status::Ok;
    }
    
    Status SendData()
    {
        if (esp_err_t err = pca9685_set_pwms(pca_handle, pwm_buffer); err != ESP_OK)
        {
            LOG_ERROR(TAG, "Failed to set PWM values with error : 0x%0x", err);
            Error::RegisterErrorEvent(ErrorEventSendDataFailed(err));
            return Status::Failure;
        }
        return Status::Ok;
    }
}