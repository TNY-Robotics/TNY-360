#include "modules/Motor.hpp"
#include "modules/AnalogScanner.hpp"
#include "modules/I2C.hpp"
#include "modules/Log.hpp"
#include "modules/NVS.hpp"
#include "config.hpp"
#include "pca9685.h"
#include <freertos/FreeRTOS.h>
#include <cmath>

namespace Motor
{
    bool initialized = false;

    float freq_min[static_cast<uint8_t>(Id::Count)] = {
        102, 102, 102, 102,
        102, 102, 102, 102,
        102, 102, 102, 102,
        102, 102, 102, 102};
    float freq_max[static_cast<uint8_t>(Id::Count)] = {
        512, 512, 512, 512,
        512, 512, 512, 512,
        512, 512, 512, 512,
        512, 512, 512, 512};
    float angle_min[static_cast<uint8_t>(Id::Count)] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    float angle_max[static_cast<uint8_t>(Id::Count)] = {
        180, 180, 180, 180,
        180, 180, 180, 180,
        180, 180, 180, 180,
        180, 180, 180, 180};

    float last_motor_freq[static_cast<uint8_t>(Id::Count)] = { // for smooth transition with duration
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    float target_motor_freq[static_cast<uint8_t>(Id::Count)] = { // all deactivated by default
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    float motor_adder[static_cast<uint8_t>(Id::Count)] = { // value to add to last_motor_freq to match movement duration
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};
    uint8_t motor_force_countdown[static_cast<uint8_t>(Id::Count)] = { // force divider for each motor (1 is full force, 5 is force 1/5 of the time)
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0};

    static uint8_t force_counter = 0; // global counter to manage force pwm simulation

    static pca9685_handle_t pca_handle;
    static NVS::Handle *nvsHandle = nullptr;

    inline AnalogScanner::Id castId(Id id)
    {
        return static_cast<AnalogScanner::Id>(static_cast<uint8_t>(id) + static_cast<uint8_t>(AnalogScanner::Id::Shoulder_FL));
    }

    Error Init()
    {
        if (initialized) return Error::Ok;

        if (I2C::Init() != Error::Ok)
        {
            Log::Add(Log::Level::Error, "Motor: Failed to initialize I2C for motor driver");
            return Error::Unknown;
        }

        // Get NVS storage for motor calibration data
        if (NVS::Open("motor_driver", &nvsHandle) != Error::Ok)
        {
            Log::Add(Log::Level::Error, "Motor: Failed to open NVS namespace");
            return Error::Unknown;
        }
        
        // TODO : Load calibration data from NVS and change freq_min, freq_range, angle_min, angle_range accordingly

        // Create PCA9685 handle
        {
            pca9685_info_t pca_info = {
                .address = MOTOR_DRIVER_I2C_ADDR,
                .clock_speed = MOTOR_DRIVER_I2C_CLOCK
            };
            esp_err_t err = pca9685_create(I2C::handle_primary, pca_info, &pca_handle);
            if (err != ESP_OK)
            {
                Log::Add(Log::Level::Error, "Motor: Failed to create PCA9685 handle");
                return Error::Unknown;
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
                Log::Add(Log::Level::Error, "Motor: Failed to configure PCA9685");
                return Error::Unknown;
            }
        }

        // Wake up the motor driver to be sure it's ready
        {
            esp_err_t err = pca9685_wake_up(pca_handle);
            if (err != ESP_OK)
            {
                Log::Add(Log::Level::Error, "Motor: Failed to wake up PCA9685");
                return Error::Unknown;
            }
        }

        initialized = true;
        return Error::Ok;
    }

    Error SetAngle(Id id, float angle)
    {
        if (id >= Id::Count)
            return Error::InvalidParameters;

        uint8_t id_idx = static_cast<uint8_t>(id);

        if (angle > std::max(angle_min[id_idx], angle_max[id_idx]) || angle < std::min(angle_min[id_idx], angle_max[id_idx]))
            return Error::InvalidParameters;

        uint16_t freq = map(angle, angle_min[id_idx], angle_max[id_idx], freq_min[id_idx], freq_max[id_idx]);
        target_motor_freq[id_idx] = freq;
        last_motor_freq[id_idx] = freq;

        return Error::Ok;
    }

    Error SetAngle(Id id, float angle, float duration_s)
    {
        if (duration_s <= 0.0f)
            return SetAngle(id, angle);

        if (id >= Id::Count)
            return Error::InvalidParameters;

        uint8_t id_idx = static_cast<uint8_t>(id);

        if (angle > std::max(angle_min[id_idx], angle_max[id_idx]) || angle < std::min(angle_min[id_idx], angle_max[id_idx]))
            return Error::InvalidParameters;

        uint16_t freq = map(angle, angle_min[id_idx], angle_max[id_idx], freq_min[id_idx], freq_max[id_idx]);
        target_motor_freq[id_idx] = freq;
        motor_adder[id_idx] = (freq - last_motor_freq[id_idx]) / (duration_s * SECONDARY_FREQUENCY_HZ);

        return Error::Ok;
    }

    Error SetAngles(const float* angles)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            SetAngle(static_cast<Id>(i), angles[i]);
        }
        return Error::Ok;
    }

    Error SetAngles(const float* angles, const float* durations_s)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            SetAngle(static_cast<Id>(i), angles[i], durations_s[i]);
        }
        return Error::Ok;
    }

    Error SetAngles(const Id* ids, const float* angles, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            SetAngle(ids[i], angles[i]);
        }
        return Error::Ok;
    }

    Error SetAngles(const Id* ids, const float* angles, const float* durations_s, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            SetAngle(ids[i], angles[i], durations_s[i]);
        }
        return Error::Ok;
    }

    // DEBUG PURPOSES ONLY
    Error SetPWM(Id id, uint16_t pwm_value)
    {
        if (id >= Id::Count)
            return Error::InvalidParameters;

        // reset motor control variables
        uint8_t id_idx = static_cast<uint8_t>(id);
        target_motor_freq[id_idx] = 0;
        last_motor_freq[id_idx] = 0;
        motor_adder[id_idx] = 0;
        motor_force_countdown[id_idx] = 0;

        // set pwm directly
        if (pca9685_set_pwm(pca_handle, id_idx, pwm_value) != ESP_OK)
        {
            Log::Add(Log::Level::Error, "Motor: Failed to set PWM for motor %d", id_idx);
            return Error::Unknown;
        }

        return Error::Ok;
    }

    Error SetForce(Id id, ForceLevel force)
    {
        if (id >= Id::Count)
            return Error::InvalidParameters;

        motor_force_countdown[static_cast<uint8_t>(id)] = static_cast<uint8_t>(force);

        return Error::Ok;
    }

    Error SetForces(const ForceLevel* forces)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            SetForce(static_cast<Id>(i), forces[i]);
        }
        return Error::Ok;
    }

    Error SetForces(const Id* ids, const ForceLevel* forces, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            SetForce(ids[i], forces[i]);
        }
        return Error::Ok;
    }

    Error GetForce(Id id, ForceLevel* outForce)
    {
        if (id >= Id::Count || outForce == nullptr)
            return Error::InvalidParameters;

        *outForce = static_cast<ForceLevel>(motor_force_countdown[static_cast<uint8_t>(id)]);

        return Error::Ok;
    }

    Error GetForces(ForceLevel* outForces)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            outForces[i] = static_cast<ForceLevel>(motor_force_countdown[i]);
        }
        return Error::Ok;
    }

    Error GetForces(const Id* ids, ForceLevel* outForces, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            outForces[i] = static_cast<ForceLevel>(motor_force_countdown[static_cast<uint8_t>(ids[i])]);
        }
        return Error::Ok;
    }

    Error GetTargetAngle(Id id, float* outAngle)
    {
        if (id >= Id::Count || outAngle == nullptr)
            return Error::InvalidParameters;

        uint8_t id_idx = static_cast<uint8_t>(id);
        *outAngle = map(target_motor_freq[id_idx], freq_min[id_idx], freq_max[id_idx], angle_min[id_idx], angle_max[id_idx]);

        return Error::Ok;
    }

    Error GetTargetAngles(float* outAngles)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            uint8_t id_idx = static_cast<uint8_t>(i);
            outAngles[i] = map(target_motor_freq[id_idx], freq_min[id_idx], freq_max[id_idx], angle_min[id_idx], angle_max[id_idx]);
        }
        return Error::Ok;
    }

    Error GetTargetAngles(const Id* ids, float* outAngles, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            uint8_t id_idx = static_cast<uint8_t>(ids[i]);
            outAngles[i] = map(target_motor_freq[id_idx], freq_min[id_idx], freq_max[id_idx], angle_min[id_idx], angle_max[id_idx]);
        }
        return Error::Ok;
    }

    Error GetCurrentAngle(Id id, float* outAngle)
    {
        return AnalogScanner::GetValue(castId(id), outAngle);
    }

    Error GetCurrentAngles(float* outAngles)
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(Id::Count); i++)
        {
            if (AnalogScanner::GetValue(castId(static_cast<Id>(i)), &outAngles[i]) != Error::Ok)
                return Error::Unknown;
        }
        return Error::Ok;
    }

    Error GetCurrentAngles(const Id* ids, float* outAngles, uint8_t count)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            if (AnalogScanner::GetValue(castId(ids[i]), &outAngles[i]) != Error::Ok)
                return Error::Unknown;
        }
        return Error::Ok;
    }
    
    void _update_task(void *pvParams)
    {
        for (int i = 0; i < static_cast<int>(Id::Count); i++)
        {
            int16_t freq_diff = target_motor_freq[i] - last_motor_freq[i];
            if (freq_diff > 0) // increase to reach target
            {
                last_motor_freq[i] = std::min(target_motor_freq[i], last_motor_freq[i] + motor_adder[i]);
            }
            if (freq_diff < 0) // decrease to reach target
            {
                last_motor_freq[i] = std::max(target_motor_freq[i], last_motor_freq[i] - motor_adder[i]);
            }

            // activate motor at force_countdown intervals to simulate force (like a low-freq force pwm)
            bool force = motor_force_countdown[i] == 0
                         ? false
                         : (force_counter % motor_force_countdown[i]) == 0;

            if (pca9685_set_pwm(pca_handle, i, (uint16_t)(force ? last_motor_freq[i] : 0)) != ESP_OK)
            {
                Log::Add(Log::Level::Error, "Motor: Failed to set PWM for motor %d", i);
            }
        }

        // increment force counter, if at 100, loop back
        force_counter++;
        if (force_counter == 100)
        {
            force_counter = 0;
        }

        // ESP_ERROR_CHECK_WITHOUT_ABORT(pca9685_set_pwms(pca_handle, target_motor_freq));
        // FIXME : The above method isn't working well, but we should use it once fixed for performance (single call vs 16 calls)
    }
}