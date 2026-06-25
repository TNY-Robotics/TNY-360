#include "locomotion/MotorController.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"
#include "locomotion/MotorController.Errors.hpp"
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

Status MotorController::init()
{
    // Check if below drivers are initialized
    LOG_SCOPE(TAG, "MotorController::init [driver=%u, reader=%u]", motor_channel, analog_channel);

    if (Status err = MotorDriver::Init(); err != Status::Ok)
    {
        state = State::ERROR;
        return err;
    }
    if (Status err = AnalogDriver::Init(); err != Status::Ok)
    {
        state = State::ERROR;
        return err;
    }

    // Try to read calibration data from NVS
    if (Status err = NVS::Init(); err != Status::Ok)
    {
        state = State::ERROR;
        return err;
    }
    char key[32]; // using motorChannel as identifier
    sprintf(key, "MtrCtrl-%d", static_cast<int>(motor_channel));
    if (Status err = NVS::Open(key, &nvshandle_ptr); err != Status::Ok)
    {
        state = State::ERROR;
        // LOG_ERROR(TAG, "Failed to open NVS handle for motor channel %d with error [%s]", motor_channel, ErrorToString(err));
        // ErrorHandle(ErrorStruct::MotorControllerInitFailed);
        return err;
    }
    if (nvshandle_ptr->get("calib_data", calibration_data) == Status::Ok)
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
    
    return Status::Ok;
}

Status MotorController::deinit()
{
    if (nvshandle_ptr)
    {
        NVS::Close(nvshandle_ptr);
        nvshandle_ptr = nullptr;
    }

    return Status::Ok;
}

Status MotorController::enable()
{
    state = State::ENABLED;
    __send_target_position(); // effective right now
    return Status::Ok;
}

Status MotorController::disable()
{
    state = State::DISABLED;
    __send_target_position(); // effective right now
    return Status::Ok;
}

Status MotorController::startCalibration()
{
    LOG_INFO(TAG, "Starting motor calibration");

    if (calibration_state == CalibrationState::CALIBRATING)
    {
        LOG_WARNING(TAG, "Motor is already in calibration mode");
        return Status::InvalidState;
    }

    if (BaseType_t err = xTaskCreatePinnedToCore([](void* param) {
            // Stop the control loop and take over for the calibration
            ControlLoop& ctrl = Robot::GetInstance().getControlLoop();
            bool wasRunning = ctrl.isRunning();
            if (wasRunning)
            {
                LOG_DEBUG(TAG, "Disabling control loop");
                if (Status err = ctrl.stop(); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Failed to stop control loop for motor calibration. Error [%s]", ErrorToString(err));
                    return;
                }
            }

            MotorController* controller = static_cast<MotorController*>(param);
            controller->calibration_state = CalibrationState::CALIBRATING;
            Status err = controller->run_calibration_sequence();
            if (err != Status::Ok)
            {
                Error::RegisterErrorEvent(ErrorEventMotorCalibrationFailed(controller->motor_channel, controller->analog_channel));
                controller->calibration_state = CalibrationState::ERROR;
            }
            else
            {
                LOG_INFO(TAG, "Motor calibrated");
                controller->calibration_state = CalibrationState::CALIBRATED;
            }
            // disable motor for safety
            if (Status err = controller->disable(); err != Status::Ok)
            {
                // LOG_ERROR(TAG, "Failed to disable motor after calibration. Error [%s]", ErrorToString(err));
            }

            // Restart the control loop (back to normal operation)
            if (wasRunning)
            {
                LOG_DEBUG(TAG, "Restarting control loop");
                if (Status err = Robot::GetInstance().getControlLoop().start(); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Failed to start control loop after motor calibration. Error [%s]", ErrorToString(err));
                    return;
                }
            }

            // clean up task handle
            vTaskDelete(nullptr);
        }, "MotorCalib", 4096, this, tskIDLE_PRIORITY + 1, &calibration_task_handle, CORE_BRAIN); err != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create motor calibration task. Error %d", err);
        return Status::Failure;
    }

    return Status::Ok;
}

Status MotorController::stopCalibration()
{
    if (calibration_state != MotorController::CalibrationState::CALIBRATING)
    {
        return Status::InvalidState;
    }

    if (calibration_task_handle != NULL)
    {
        vTaskDelete(calibration_task_handle);
        deleteCalibrationData(false); // reset calibration data but don't save to NVS
        LOG_WARNING(TAG, "Stopped motor calibration. This could lead to unexpected behavior.");
    }

    return Status::Ok;
}

Status MotorController::setCalibrationData(CalibrationData& data, bool save)
{
    calibration_data = data;
    calibration_state = CalibrationState::CALIBRATED;
    if (save)
    {
        if (Status err = save_calibration_data(); err != Status::Ok)
        {
            return err;
        }
    }
    return Status::Ok;
}

Status MotorController::deleteCalibrationData(bool save)
{
    calibration_state = CalibrationState::UNCALIBRATED;
    calibration_data.dc_min = motor_attributes.dc_min;
    calibration_data.dc_max = motor_attributes.dc_max;
    if (save)
    {
        if (Status err = delete_calibration_data(); err != Status::Ok)
        {
            return err;
        }
    }
    return Status::Ok;
}

Status MotorController::setCalibrationState(CalibrationState state)
{
    LOG_WARNING(TAG, "Manually changing calibration state to [%d]. This could lead to unexpected behavior.", static_cast<uint8_t>(state));
    calibration_state = state;
    return Status::Ok;
}

Status MotorController::setTargetPosition(float position)
{
    // clamp position between 0.0 and 1.0
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;

    // set target position
    target_position = position;
    if (state == State::ENABLED)
    {
        if (Status err = __send_target_position(); err != Status::Ok)
        {
            return err;
        }
    }

    return Status::Ok;
}

Status MotorController::getTargetPosition(float& result) const
{
    result = target_position;
    return Status::Ok;
}

Status MotorController::getCurrentPosition(float& result) const
{
    if (!motor_attributes.has_feedback || calibration_state != CalibrationState::CALIBRATED)
    {
        result = target_position;
        return Status::Ok;
    }

    AnalogDriver::Value voltage_mV = 0;
    if (Status err = AnalogDriver::GetVoltage(analog_channel, voltage_mV); err != Status::Ok)
    {
        return err;
    }
    float position = (static_cast<float>(voltage_mV) - calibration_data.feedback_min) /
                     (calibration_data.feedback_max - calibration_data.feedback_min);

    result = position;
    return Status::Ok;
}

Status MotorController::__send_target_position()
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
        if (Status err = MotorDriver::SetDutyCycle(motor_channel, dc_value); err != Status::Ok)
        {
            return err;
        }
    }

    return Status::Ok;
}

Status MotorController::save_calibration_data()
{
    if (Status err = nvshandle_ptr->set("calib_data", calibration_data); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Failed to save calibration data to NVS with error [%s]", ErrorToString(err));
        return err;
    }
    return Status::Ok;
}

Status MotorController::delete_calibration_data()
{
    if (Status err = nvshandle_ptr->erase("calib_data"); err != Status::Ok)
    {
        // LOG_ERROR(TAG, "Failed to delete calibration data from NVS with error [%s]", ErrorToString(err));
        return err;
    }
    return Status::Ok;
}
