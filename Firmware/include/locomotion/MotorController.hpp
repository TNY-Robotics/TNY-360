#pragma once
#include "drivers/MotorDriver.hpp"
#include "drivers/AnalogDriver.hpp"
#include "common/NVS.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class MotorController
{
public:
    enum State
    {
        ENABLED,
        DISABLED,
        ERROR
    };

    enum CalibrationState
    {
        UNCALIBRATED,
        CALIBRATING,
        CALIBRATED
    };

    MotorController();
    
    MotorController(MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel);

    /**
     * @brief Initialize the motor controller.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the motor controller.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Turns on the motor.
     * @return Error code indicating success or failure.
     */
    Error enable();

    /**
     * @brief Turns off the motor.
     * @return Error code indicating success or failure.
     */
    Error disable();

    /**
     * @brief Starts the calibration process.
     * @return Error code indicating success or failure.
     */
    Error startCalibration();

    /**
     * @brief Stops the calibration process (no saving).
     * @note This method should not be called to save calibration data, it is done automatically on calibration sequence completion.
     * @return Error code indicating success or failure.
     */
    Error stopCalibration();

    /**
     * @brief Set the target position for the motor.
     * @param position Target position as a ratio (0.0 to 1.0).
     * @note An approximate position can be set even if the motor is uncalibrated.
     * @return Error code indicating success or failure.
     */
    Error setTargetPosition(float position);

    /**
     * @brief Get the target position of the motor.
     * @param result Target position as a ratio (0.0 to 1.0).
     * @return Error code indicating success or failure.
     */
    Error getTargetPosition(float& result) const;

    /**
     * @brief Get the current position of the motor.
     * @param result Current position as a ratio (0.0 to 1.0).
     * @return Error code indicating success or failure.
     * @note An approximate position will be returned until the motor is calibrated.
     */
    Error getCurrentPosition(float& result) const;

    /**
     * @brief Get the current state of the motor controller.
     * @return Current state.
     */
    State getState() const;

    /**
     * @brief Get the current state of calibration.
     * @return Current calibration state.
     */
    CalibrationState getCalibrationState() const { return calibration_state; }

    /**
     * @brief Get the motor channel.
     * @return Motor channel.
     */
    MotorDriver::Channel getMotorChannel() const { return motor_channel; }

    /**
     * @brief Get the analog channel.
     * @return Analog channel.
     */
    AnalogDriver::Channel getAnalogChannel() const { return analog_channel; }

private:
    struct CalibrationData
    {
        MotorDriver::Value min_pwm;
        MotorDriver::Value max_pwm;
        AnalogDriver::Value min_voltage;
        AnalogDriver::Value max_voltage;
        AnalogDriver::Value feedback_noise;
        MotorDriver::Value pwm_deadband;
        uint16_t feedback_latency_ms;
    };

    static constexpr CalibrationData DEFAULT_CALIBRATION = {
        .min_pwm = static_cast<uint16_t>(4096 * 0.5f) / 20, // 0.5us duty cycle at 50Hz in 12-bit
        .max_pwm = static_cast<uint16_t>(4096 * 2.5f) / 20, // 2.5us duty cycle at 50Hz in 12-bit
        .min_voltage = 200,  // in mV
        .max_voltage = 3000, // in mV
        .feedback_noise = 1, // in mV
        .pwm_deadband = 1, // in PWM units
        .feedback_latency_ms = 0 // in ms
    };

    MotorDriver::Channel motor_channel;
    AnalogDriver::Channel analog_channel;
    NVS::Handle* nvshandle_ptr;
    CalibrationData calibration_data;
    float target_position;
    State state;
    CalibrationState calibration_state;

    TaskHandle_t calibration_task_handle;

    Error __send_target_position();

    void abort_calibration();
    Error run_calibration_sequence();
    Error detect_position_feedback_noise(AnalogDriver::Value* out_noise_level_mV);
    Error detect_feedback_latency(AnalogDriver::Value noise_level_mV, TickType_t* out_latency_ticks);
    Error detect_servo_pwm_deadband(MotorDriver::Value default_position, AnalogDriver::Value feedback_noise_level_mV, MotorDriver::Value* out_deadband_pwm);
};