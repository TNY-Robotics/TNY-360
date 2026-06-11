#include "locomotion/MotorController.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"
#include <cstdio>

constexpr const char* TAG = "MotorController";

MotorController::MotorController() {}

MotorController::MotorController(MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel, MotorAttributes attribs)
    : motor_channel(motor_channel),
      analog_channel(analog_channel),
      motor_attributes(attribs),
      calibration_state(CalibrationState::UNCALIBRATED),
      state(State::DISABLED),
      target_position(0.5f),
      nvshandle_ptr(nullptr)
{
    // set default pwm bounds to motor attributes
    calibration_data.dc_min = motor_attributes.dc_min;
    calibration_data.dc_max = motor_attributes.dc_max;
    disable(); // make sure motor is off by default
}

Error MotorController::init()
{
    // Check if below drivers are initialized
    LOG_SCOPE(TAG, "MotorController::init [driver=%u, reader=%u]", motor_channel, analog_channel);

    if (Error err = MotorDriver::Init(); err != Error::None)
    {
        state = State::ERROR;
        return err;
    }
    if (Error err = AnalogDriver::Init(); err != Error::None)
    {
        state = State::ERROR;
        return err;
    }

    // Try to read calibration data from NVS
    if (Error err = NVS::Init(); err != Error::None)
    {
        state = State::ERROR;
        return err;
    }
    char key[32]; // using motorChannel as identifier
    sprintf(key, "MtrCtrl-%d", static_cast<int>(motor_channel));
    if (Error err = NVS::Open(key, &nvshandle_ptr); err != Error::None)
    {
        state = State::ERROR;
        LOG_ERROR(TAG, "Failed to open NVS handle for motor channel %d with error [%s]", motor_channel, ErrorToString(err));
        ErrorHandle(ErrorStruct::MotorControllerInitFailed);
        return err;
    }
    if (nvshandle_ptr->get("calib_data", calibration_data) == Error::None)
    {
        LOG_INFO(TAG, "Calibration data loaded from NVS for motor channel %d", motor_channel);
        state = State::ENABLED;
        calibration_state = CalibrationState::CALIBRATED;
    }
    else
    {
        LOG_WARNING(TAG, "No calibration data found");
        calibration_state = CalibrationState::UNCALIBRATED;
    }

    // Disable motor initially
    disable();
    
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
    state = State::ENABLED;
    __send_target_position(); // effective right now
    return Error::None;
}

Error MotorController::disable()
{
    state = State::DISABLED;
    __send_target_position(); // effective right now
    return Error::None;
}

Error MotorController::startCalibration()
{
    LOG_INFO(TAG, "Starting motor calibration");

    if (calibration_state == CalibrationState::CALIBRATING)
    {
        LOG_WARNING(TAG, "Motor is already in calibration mode");
        return Error::InvalidState;
    }

    if (BaseType_t err = xTaskCreatePinnedToCore([](void* param) {
            // Stop the control loop and take over for the calibration
            ControlLoop& ctrl = Robot::GetInstance().getControlLoop();
            bool wasRunning = ctrl.isRunning();
            if (wasRunning)
            {
                LOG_DEBUG(TAG, "Disabling control loop");
                if (Error err = ctrl.stop(); err != Error::None)
                {
                    LOG_ERROR(TAG, "Failed to stop control loop for motor calibration. Error [%s]", ErrorToString(err));
                    return;
                }
            }

            MotorController* controller = static_cast<MotorController*>(param);
            controller->calibration_state = CalibrationState::CALIBRATING;
            Error err = controller->run_calibration_sequence();
            if (err != Error::None)
            {
                LOG_ERROR(TAG, "Motor calibration with error [%s]", ErrorToString(err));
                controller->calibration_state = CalibrationState::ERROR;
            }
            else
            {
                LOG_INFO(TAG, "Motor calibrated");
                controller->calibration_state = CalibrationState::CALIBRATED;
            }
            // disable motor for safety
            if (Error err = controller->disable(); err != Error::None)
            {
                LOG_ERROR(TAG, "Failed to disable motor after calibration. Error [%s]", ErrorToString(err));
            }

            // Restart the control loop (back to normal operation)
            if (wasRunning)
            {
                LOG_DEBUG(TAG, "Restarting control loop");
                if (Error err = Robot::GetInstance().getControlLoop().start(); err != Error::None)
                {
                    LOG_ERROR(TAG, "Failed to start control loop after motor calibration. Error [%s]", ErrorToString(err));
                    return;
                }
            }

            // clean up task handle
            vTaskDelete(nullptr);
        }, "MotorCalib", 4096, this, tskIDLE_PRIORITY + 1, &calibration_task_handle, CORE_BRAIN); err != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create motor calibration task. Error %d", err);
        return Error::SoftwareFailure;
    }

    return Error::None;
}

Error MotorController::stopCalibration()
{
    if (calibration_state != MotorController::CalibrationState::CALIBRATING)
    {
        return Error::InvalidState;
    }

    if (calibration_task_handle != NULL)
    {
        vTaskDelete(calibration_task_handle);
        deleteCalibrationData(false); // reset calibration data but don't save to NVS
        LOG_WARNING(TAG, "Stopped motor calibration. This could lead to unexpected behavior.");
    }

    return Error::None;
}

Error MotorController::setCalibrationData(CalibrationData& data, bool save)
{
    calibration_data = data;
    calibration_state = CalibrationState::CALIBRATED;
    if (save)
    {
        if (Error err = save_calibration_data(); err != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}

Error MotorController::deleteCalibrationData(bool save)
{
    calibration_state = CalibrationState::UNCALIBRATED;
    calibration_data.dc_min = motor_attributes.dc_min;
    calibration_data.dc_max = motor_attributes.dc_max;
    if (save)
    {
        if (Error err = delete_calibration_data(); err != Error::None)
        {
            return err;
        }
    }
    return Error::None;
}

Error MotorController::setCalibrationState(CalibrationState state)
{
    LOG_WARNING(TAG, "Manually changing calibration state to [%d]. This could lead to unexpected behavior.", static_cast<uint8_t>(state));
    calibration_state = state;
    return Error::None;
}

Error MotorController::setTargetPosition(float position)
{
    // clamp position between 0.0 and 1.0
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;

    // set target position
    target_position = position;
    if (state == State::ENABLED)
    {
        if (Error err = __send_target_position(); err != Error::None)
        {
            return err;
        }
    }

    return Error::None;
}

Error MotorController::getTargetPosition(float& result) const
{
    result = target_position;
    return Error::None;
}

Error MotorController::getCurrentPosition(float& result) const
{
    if (!motor_attributes.has_feedback || calibration_state != CalibrationState::CALIBRATED)
    {
        result = target_position;
        return Error::None;
    }

    AnalogDriver::Value voltage_mV = 0;
    if (Error err = AnalogDriver::GetVoltage(analog_channel, voltage_mV); err != Error::None)
    {
        return err;
    }
    float position = (static_cast<float>(voltage_mV) - calibration_data.feedback_min) /
                     (calibration_data.feedback_max - calibration_data.feedback_min);

    result = position;
    return Error::None;
}

Error MotorController::__send_target_position()
{
    MotorDriver::Value dc_value = 0;
    if (state == State::ENABLED)
    {
        dc_value = static_cast<MotorDriver::Value>(
            calibration_data.dc_min +
            target_position * (calibration_data.dc_max - calibration_data.dc_min)
        );
    }

    if (calibration_state != CalibrationState::CALIBRATING) // don't listen to anyone if calibrating
    {
        if (Error err = MotorDriver::SetDutyCycle(motor_channel, dc_value); err != Error::None)
        {
            return err;
        }
    }

    return Error::None;
}

Error MotorController::save_calibration_data()
{
    if (Error err = nvshandle_ptr->set("calib_data", calibration_data); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to save calibration data to NVS with error [%s]", ErrorToString(err));
        return err;
    }
    return Error::None;
}

Error MotorController::delete_calibration_data()
{
    if (Error err = nvshandle_ptr->erase("calib_data"); err != Error::None)
    {
        LOG_ERROR(TAG, "Failed to delete calibration data from NVS with error [%s]", ErrorToString(err));
        return err;
    }
    return Error::None;
}
