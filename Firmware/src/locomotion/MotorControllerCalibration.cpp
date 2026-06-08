#include "locomotion/MotorController.hpp"
#include "common/Log.hpp"
#include "locomotion/Joint.hpp"
#include "locomotion/calibration/get_feedback_noise.hpp"
#include "locomotion/calibration/check_feedback_inversion.hpp"
#include "locomotion/calibration/get_feedback_latency.hpp"
#include "locomotion/calibration/get_deadband_size.hpp"
#include "locomotion/calibration/get_physical_bound.hpp"

constexpr const char* TAG = "MtrCtrl-Calib";

Error MotorController::run_calibration_sequence()
{
    MotorDriver::Value pwm_center = (motor_attributes.pwm_min + motor_attributes.pwm_max) / 2;
    LOG_DEBUG(TAG, "PWM center is %d", pwm_center);

    AnalogDriver::Value noise_v;
    bool feedback_inverted;
    float feedback_latency_ms;
    float deadband_size;
    BoundDescription min_bound;
    BoundDescription max_bound;

    LOG_SCOPE(TAG, "Starting Motor Calibration [motor_channel=%d, analog_channel=%d]", motor_channel, analog_channel);
    this->calibration_progress = 0.0f;

    // Disable all motors
    LOG_DEBUG(TAG, "Disabling all motors");
    {
        for (int i = 0; i < (int) Joint::Id::Count; i++)
        {
            Joint* joint = Joint::GetJoint((Joint::Id)i);
            if (joint) joint->disable();
        }
        RETURN_ERROR(MotorDriver::DisableAllMotors());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    this->calibration_progress = 0.1f;

    // Move motor to center
    LOG_DEBUG(TAG, "Moving to center");
    {
        RETURN_ERROR(MotorDriver::SetPWM(motor_channel, pwm_center));
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    this->calibration_progress = 0.2f;

    // Get motor feedback noise
    LOG_DEBUG(TAG, "Getting feedback noise");
    {
        FeedbackNoiseParams params;
        if (Error err = get_feedback_noise(params, analog_channel, noise_v); err != Error::None)
        {
            LOG_ERROR(TAG, "Feedback noise detection failed with error [%s]", ErrorToString(err));
            return Error::Unknown;
        }
        LOG_DEBUG(TAG, "==> Feedback noise is %.3f V", noise_v);
    }
    this->calibration_progress = 0.3f;

    // Check feedback inversion
    LOG_DEBUG(TAG, "Checking feedback inversion");
    {
        FeedbackInversionParams params;
        params.pwm_center = pwm_center;
        params.pwm_delta = MotorDriver::MS_TO_PWM(0.2);
        params.feedback_noise = noise_v;
        if (Error err = check_feedback_inversion(params, motor_channel, analog_channel, feedback_inverted); err != Error::None)
        {
            LOG_ERROR(TAG, "Feedback invertion detection failed with error [%s]", ErrorToString(err));
            return Error::Unknown;
        }
        LOG_DEBUG(TAG, "==> Feedback inverted : %s", feedback_inverted? "true": "false");
    }
    this->calibration_progress = 0.4f;

    // Test feedback latency
    LOG_DEBUG(TAG, "Estimating feedback latency");
    {
        FeedbackLatencyParams params;
        params.pwm_center = pwm_center;
        params.pwm_delta = MotorDriver::MS_TO_PWM(0.2);
        params.feedback_noise = noise_v;
        params.nb_samples = 6;
        params.nb_subsamples = 20;
        if (Error err = get_feedback_latency(params, motor_channel, analog_channel, feedback_latency_ms); err != Error::None)
        {
            LOG_ERROR(TAG, "Feedback latency estimation failed with error [%s]", ErrorToString(err));
            return Error::Unknown;
        }
        LOG_DEBUG(TAG, "==> Feedback latency : %.1f ms", feedback_latency_ms);
    }
    this->calibration_progress = 0.5f;

    // Test deadband size
    LOG_DEBUG(TAG, "Testing deadband size");
    {
        if  (noise_v > 0.015f) // If noise > 15mV, hardcode deadband because we won't be able to detect it properly
        {
            LOG_WARNING(TAG, "Feedback noise is quite high (%.3f V). Hardcoding deadband size", noise_v);
            deadband_size = MotorDriver::MS_TO_PWM(0.01); // around 1 degree, which is a common value for servomotors.
        }
        else
        {
            DeadbandSizeParams params;
            params.pwm_center = pwm_center;
            params.max_pwm = MotorDriver::MS_TO_PWM(2.0);
            params.feedback_noise = noise_v;
            if (Error err = get_deadband_size(params, motor_channel, analog_channel, deadband_size); err != Error::None)
            {
                LOG_ERROR(TAG, "Deadband size estimation failed with error [%s]", ErrorToString(err));
                return Error::Unknown;
            }
        }
        LOG_DEBUG(TAG, "==> Deadband size : %.1f PWM units (%.2f ms, %.2f deg)", deadband_size, MotorDriver::PWM_TO_MS(deadband_size), MotorDriver::PWM_TO_MS(deadband_size) * 90.f);
    }
    this->calibration_progress = 0.6f;

    // Test min bound
    LOG_DEBUG(TAG, "Detecting minimum bound");
    {
        PhysicalBoundParams params;
        params.pwm_center = pwm_center;
        params.pwm_max = MotorDriver::MS_TO_PWM(2.9);
        params.pwm_min = MotorDriver::MS_TO_PWM(0.1);
        params.pwm_safe = MotorDriver::MS_TO_PWM(0.3);
        params.direction = -1;
        if (Error err = get_physical_bound(params, motor_channel, analog_channel, min_bound); err != Error::None)
        {
            LOG_ERROR(TAG, "Minimum bound detection failed with error [%s]", ErrorToString(err));
            return Error::Unknown;
        }
        LOG_DEBUG(TAG, "==> Minimum bound : %d PWM and %.2f V", min_bound.pwm_value, min_bound.feedback_value);
    }
    this->calibration_progress = 0.7f;

    // Test max bound
    LOG_DEBUG(TAG, "Detecting maximum bound");
    {
        PhysicalBoundParams params;
        params.pwm_center = pwm_center;
        params.pwm_max = MotorDriver::MS_TO_PWM(2.9);
        params.pwm_min = MotorDriver::MS_TO_PWM(0.1);
        params.pwm_safe = MotorDriver::MS_TO_PWM(0.3);
        params.direction = 1;
        if (Error err = get_physical_bound(params, motor_channel, analog_channel, max_bound); err != Error::None)
        {
            LOG_ERROR(TAG, "Maximum bound detection failed with error [%s]", ErrorToString(err));
            return Error::Unknown;
        }
        LOG_DEBUG(TAG, "==> Maximum bound : %d PWM and %.2f V", max_bound.pwm_value, max_bound.feedback_value);
    }
    this->calibration_progress = 0.8f;

    // Test max speed
    LOG_DEBUG(TAG, "Testing maximum speed");
    {
        // TODO : Implement max speed estimation (by quickly moving between the two bounds and measuring the time)
    }
    this->calibration_progress = 0.9f;

    LOG_DEBUG(TAG, "Calibration complete, centering motor before disabling it");
    {
        RETURN_ERROR(MotorDriver::SetPWM(motor_channel, pwm_center))
        RETURN_ERROR(MotorDriver::SendData());
        vTaskDelay(pdMS_TO_TICKS(800));
        RETURN_ERROR(MotorDriver::DisableAllMotors());
    }

    // Save calibration data
    LOG_DEBUG(TAG, "Saving calibration data");
    {
        CalibrationData data;
        data.pwm_min = min_bound.pwm_value;
        data.pwm_max = max_bound.pwm_value;
        data.pwm_deadband = deadband_size;
        data.feedback_min = min_bound.feedback_value;
        data.feedback_max = max_bound.feedback_value;
        data.feedback_noise = noise_v;
        data.feedback_inverted = feedback_inverted;
        data.feedback_latency_ms = feedback_latency_ms;
        data.max_speed = 0.f; // TODO: implement max speed estimation

        this->calibration_data = data;
        save_calibration_data();
    }
    this->calibration_progress = 1.0f;

    return Error::None;
}