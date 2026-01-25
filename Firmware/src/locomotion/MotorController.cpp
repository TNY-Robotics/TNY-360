#include "locomotion/MotorController.hpp"
#include "common/Log.hpp"
#include <cstdio>

constexpr const char* TAG = "MotorController";

MotorController::MotorController() {}

MotorController::MotorController(MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel)
    : motor_channel(motor_channel),
      analog_channel(analog_channel),
      nvshandle_ptr(nullptr),
      target_position(0.0f),
      calibration_state(UNCALIBRATED)
{
}

Error MotorController::init()
{
    // Check if below drivers are initialized
    if (Error err = MotorDriver::Init(); err != Error::None)
    {
        state = ERROR;
        return err;
    }
    if (Error err = AnalogDriver::Init(); err != Error::None)
    {
        state = ERROR;
        return err;
    }

    // Try to read calibration data from NVS
    if (Error err = NVS::Init(); err != Error::None)
    {
        state = ERROR;
        return err;
    }
    char key[32]; // using motorChannel as identifier
    sprintf(key, "MtrCtrl-%d", static_cast<int>(motor_channel));
    if (Error err = NVS::Open(key, &nvshandle_ptr); err != Error::None)
    {
        state = ERROR;
        return err;
    }
    if (nvshandle_ptr->get("calib_data", calibration_data) == Error::None)
    {
        state = ENABLED;
        calibration_state = CALIBRATED;
    }
    else
    {
        calibration_data = DEFAULT_CALIBRATION;
        calibration_state = UNCALIBRATED;
    }

    // Disable motor initially
    state = DISABLED;
    __send_target_position();
    
    return Error::None;
}

Error MotorController::deinit()
{
    if (nvshandle_ptr)
    {
        NVS::Close(nvshandle_ptr);
        nvshandle_ptr = nullptr;
    }

    return Error::None;
}

Error MotorController::enable()
{
    state = ENABLED;
    __send_target_position(); // effective right now
    return Error::None;
}

Error MotorController::disable()
{
    state = DISABLED;
    __send_target_position(); // effective right now
    return Error::None;
}

Error MotorController::startCalibration()
{
    Log::Add(Log::Level::Info, TAG, "Starting motor calibration");

    if (calibration_state == CALIBRATING)
    {
        Log::Add(Log::Level::Warning, TAG, "Motor is already in calibration mode");
        return Error::InvalidState;
    }

    if (xTaskCreate([](void* param) {
            MotorController* controller = static_cast<MotorController*>(param);
            Error err = controller->run_calibration_sequence();
            if (err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Motor calibration task failed with error %d", static_cast<uint8_t>(err));
            }
            else
            {
                Log::Add(Log::Level::Info, TAG, "Motor calibration task completed successfully");
            }
            // disable motor for safety
            if (Error err = controller->disable(); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to disable motor after calibration. Error: %d", static_cast<uint8_t>(err));
            }
            // clean up task handle
            vTaskDelete(nullptr);
        }, "MotorCalib", 4096, this, tskIDLE_PRIORITY + 1, &calibration_task_handle) != pdPASS)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to create motor calibration task");
        return Error::SoftwareFailure;
    }

    return Error::None;
}

Error MotorController::stopCalibration()
{
    Log::Add(Log::Level::Info, TAG, "Stopping motor calibration");

    if (calibration_state != CALIBRATING)
    {
        Log::Add(Log::Level::Warning, TAG, "Motor is not in calibration mode");
        return Error::InvalidState;
    }

    return Error::None;
}

Error MotorController::setTargetPosition(float position)
{
    // clamp position between 0.0 and 1.0
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;

    // set target position
    target_position = position;
    __send_target_position();

    return Error::None;
}

Error MotorController::getTargetPosition(float& result) const
{
    result = target_position;
    return Error::None;
}

Error MotorController::getCurrentPosition(float& result) const
{
    AnalogDriver::Value voltage_mV = 0;
    if (Error err = AnalogDriver::GetVoltage(analog_channel, &voltage_mV); err != Error::None)
    {
        return err;
    }
    float position = (static_cast<float>(voltage_mV) - calibration_data.min_voltage) /
                     (calibration_data.max_voltage - calibration_data.min_voltage);
    
    result = position;
    return Error::None;
}

MotorController::State MotorController::getState() const
{
    return state;
}


Error MotorController::__send_target_position()
{
    MotorDriver::Value pwm_value = 0;
    if (state != DISABLED && state != ERROR)
    {
        pwm_value = static_cast<MotorDriver::Value>(
            calibration_data.min_pwm +
            target_position * (calibration_data.max_pwm - calibration_data.min_pwm)
        );
    }

    if (calibration_state != CALIBRATING) // don't listen to anyone if calibrating
    {
        // Log::Add(Log::Level::Debug, TAG, "Setting motor %d PWM to %d", motor_channel, pwm_value);
        if (Error err = MotorDriver::SetPWM(motor_channel, pwm_value); err != Error::None)
        {
            return err;
        }
    }

    return Error::None;
}
