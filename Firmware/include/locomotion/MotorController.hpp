#pragma once
#include "drivers/MotorDriver.hpp"
#include "drivers/AnalogDriver.hpp"
#include "common/NVS.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class MotorController
{
public:
    enum class State : uint8_t
    {
        ENABLED = 1,
        DISABLED = 0,
        ERROR = 2
    };

    enum class CalibrationState : uint8_t
    {
        UNCALIBRATED = 0,
        CALIBRATING = 1,
        CALIBRATED = 2
    };

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

    constexpr static CalibrationData DEFAULT_CALIBRATION_MG996R = {
        .min_pwm = MotorDriver::MS_TO_PWM(0.5f), // 0.5us duty cycle
        .max_pwm = MotorDriver::MS_TO_PWM(2.5f), // 2.5us duty cycle
        .min_voltage = 200,  // in mV
        .max_voltage = 3000, // in mV
        .feedback_noise = 1, // in mV
        .pwm_deadband = 1, // in PWM units
        .feedback_latency_ms = 0 // in ms
    };

    constexpr static CalibrationData DEFAULT_CALIBRATION_SG90 = {
        .min_pwm = MotorDriver::MS_TO_PWM(1.0f), // 1.0us duty cycle
        .max_pwm = MotorDriver::MS_TO_PWM(2.0f), // 2.0us duty cycle
        .min_voltage = 0,  // in mV | NOTE : NO FEEDBACK ON EARS (SG90)
        .max_voltage = 0, // in mV  | NOTE : NO FEEDBACK ON EARS (SG90)
        .feedback_noise = 0, // in mV
        .pwm_deadband = 0, // in PWM units
        .feedback_latency_ms = 0 // in ms
    };

    MotorController();
    
    MotorController(MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel);
    
    MotorController(MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, CalibrationData default_calibration);

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
     * @brief Get the calibration progress as a ratio (0.0 to 1.0).
     * @return Calibration progress.
     */
    float getCalibrationProgress() const { return calibration_progress; }

    /**
     * @brief Set the calibration state.
     * @param state New calibration state.
     * @return Error code indicating success or failure.
     * @note Should only be used for manual calibration control. 
     */
    Error setCalibrationState(CalibrationState state);

    /**
     * @brief Get the current calibration data
     * @return Current calibration data
     */
    const CalibrationData& getCalibrationData() const { return calibration_data; }

    /**
     * @brief Set the current calibration data
     * @param data The CalibrationData object to set
     * @param save Should the data be saved in NVS for future use
     * @return If the save was successfull or not
     */
    Error setCalibrationData(CalibrationData& data, bool save = true);

    /**
     * @brief Delete the current calibration data (fallback to default)
     * @param save Should the deletion be saved in NVS or not
     * @return If the deletion was successfull or not
     */
    Error deleteCalibrationData(bool save = true);

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
    State getState() const { return state; }

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
    MotorDriver::Channel motor_channel;
    AnalogDriver::Channel analog_channel;
    NVS::Handle* nvshandle_ptr;
    CalibrationData calibration_data;
    CalibrationData default_calibration_data;
    float target_position;
    State state;
    CalibrationState calibration_state;
    float calibration_progress;

    TaskHandle_t calibration_task_handle;

    Error __send_target_position();

    Error save_calibration_data();

    void abort_calibration();
    Error run_calibration_sequence();
    Error detect_position_feedback_noise(AnalogDriver::Value* out_noise_level_mV);
    Error detect_feedback_latency(AnalogDriver::Value noise_level_mV, TickType_t* out_latency_ticks);
    Error detect_servo_pwm_deadband(MotorDriver::Value default_position, AnalogDriver::Value feedback_noise_level_mV, MotorDriver::Value* out_deadband_pwm);
};