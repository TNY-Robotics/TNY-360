#include "locomotion/MotorCalibration.hpp"
#include "locomotion/MotorCalibration/get_feedback_noise.hpp"
#include "locomotion/MotorCalibration/check_feedback_inversion.hpp"
#include "locomotion/MotorCalibration/get_feedback_latency.hpp"
#include "locomotion/MotorCalibration/get_deadband_size.hpp"
#include "locomotion/MotorCalibration/get_physical_bound.hpp"
#include "locomotion/MotorCalibration.Errors.hpp"
#include "locomotion/Joint.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"

constexpr const char* TAG = "MtrCalib";

constexpr float DC_MIN = 0.5f; // min duty cycle for servomotors (0.5ms pulse width is common for 0° rotation)
constexpr float DC_MAX = 2.5f; // max duty cycle for servomotors (2.5ms pulse width is common for 180° rotation)
constexpr float ANGLE_MIN = 0.0f; // min angle for servomotors (0° in radians)
constexpr float ANGLE_MAX = 3.14159f; // max angle for servomotors (180° in radians)

namespace MotorCalibration
{
    namespace internal
    {
        bool initialized = false;

        CalibrationData calibration_data_registry[(int)Joint::Id::Count];
        JointCalibrationState calibration_state_registry[(int)Joint::Id::Count];
        bool calibration_data_loaded[(int)Joint::Id::Count] = { false };

        CalibrationState current_calibration_state;
        TaskHandle_t calibration_task_handle;
        NVS::Handle* nvshandle_ptr;

        /// @brief Creates a default *valid but not safe* calibration data for a joint based on its configuration.
        /// @param config Joint configuration
        /// @return CalibrationData object with default values
        CalibrationData create_default_calibration(JointConfig config)
        {
            const float fb_0d = config.has_feedback ? 0.5f : 0.0f;
            const float fb_180d = config.has_feedback ? 2.8f : 0.0f;
            const float dc_0d = DC_MIN;
            const float dc_180d = DC_MAX;
            const float min_stop_angle_ratio = (config.min_angle_rad - ANGLE_MIN) / (ANGLE_MAX - ANGLE_MIN);
            const float max_stop_angle_ratio = (config.max_angle_rad - ANGLE_MIN) / (ANGLE_MAX - ANGLE_MIN);

            return CalibrationData{
                .dc_min = dc_0d + min_stop_angle_ratio * (dc_180d-dc_0d),
                .dc_max = dc_0d + max_stop_angle_ratio * (dc_180d-dc_0d),
                .dc_deadband = 0.01f, // around 1° of deadband
                .fb_min = fb_0d + min_stop_angle_ratio * (fb_180d-fb_0d),
                .fb_max = fb_0d + max_stop_angle_ratio * (fb_180d-fb_0d),
                .fb_noise = 0.01f, // around 1° of noise
                .fb_latency_ms = 0,
                .fb_inverted = false,
                .max_speed = 6.0f
            };
        }
        
        Status save_calibration_data(Joint::Id joint)
        {
            char key[16];
            sprintf(key, "cbdt_%d", (int)joint); // calib data [number]

            const CalibrationData& calibration_data = internal::calibration_data_registry[(int)joint];
            if (Status err = nvshandle_ptr->set(key, calibration_data); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Failed to save calibration data to NVS with error [%s]", StatusToString(err));
                return err;
            }

            return Status::Ok;
        }
        
        Status load_calibration_data(Joint::Id joint)
        {
            char key[16];
            sprintf(key, "cbdt_%d", (int)joint); // calib data [number]

            CalibrationData& calibration_data = internal::calibration_data_registry[(int)joint];
            if (Status err = nvshandle_ptr->get(key, calibration_data); err != Status::Ok)
            {
                LOG_WARNING(TAG, "Failed to load calibration data from NVS with error [%s]. Using default values.", StatusToString(err));
                Joint* joint_ptr = Joint::GetJoint(joint);
                if (!joint_ptr)
                {
                    LOG_ERROR(TAG, "Failed to get joint pointer for joint [%d]. Cannot load default calibration data.", (int)joint);
                    return Status::NotFound;
                }
                calibration_data = create_default_calibration(joint_ptr->getConfig());
                calibration_state_registry[(int)joint] = JointCalibrationState::UNCALIBRATED;
            }
            else
            {
                calibration_state_registry[(int)joint] = JointCalibrationState::CALIBRATED;
            }

            return Status::Ok;
        }

        Status delete_calibration_data(Joint::Id joint)
        {
            char key[16];
            sprintf(key, "cbdt_%d", (int)joint); // calib data [number]
            if (Status err = nvshandle_ptr->erase(key); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Failed to delete calibration data from NVS with error [%s]", StatusToString(err));
                return err;
            }

            return Status::Ok;
        }

        Status run_calibration_sequence(CalibrationData& calibration_data, MotorDriver::Channel motor_channel, AnalogDriver::Channel analog_channel)
        {
            MotorDriver::Value dc_center = (DC_MIN + DC_MAX) / 2.0f;

            AnalogDriver::Value noise_v;
            bool fb_inverted;
            float fb_latency_ms;
            float db_size;
            BoundDescription min_bound;
            BoundDescription max_bound;

            LOG_SCOPE(TAG, "Starting Motor Calibration [motor_channel=%d, analog_channel=%d]", motor_channel, analog_channel);
            current_calibration_state.progress = 0.0f;

            // Disable all motors
            LOG_DEBUG(TAG, "Disabling all motors");
            {
                for (int i = 0; i < (int) Joint::Id::Count; i++)
                {
                    Joint* joint = Joint::GetJoint((Joint::Id)i);
                    if (joint) joint->disable();
                }
                RETURN_ON_ERROR(MotorDriver::DisableAllMotors());
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            internal::current_calibration_state.progress = 0.1f;

            // Move motor to center
            LOG_DEBUG(TAG, "Moving to center (duty cycle: %.2f)", dc_center);
            {
                RETURN_ON_ERROR(MotorDriver::SetDutyCycle(motor_channel, dc_center));
                RETURN_ON_ERROR(MotorDriver::SendData());
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            internal::current_calibration_state.progress = 0.2f;

            // Get motor feedback noise
            LOG_DEBUG(TAG, "Getting feedback noise");
            {
                FeedbackNoiseParams params;
                if (Status err = get_feedback_noise(params, analog_channel, noise_v); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Feedback noise detection failed with error [%s]", ErrorToString(err));
                    return Status::Unknown;
                }
                LOG_DEBUG(TAG, "==> Feedback noise is %.3f V", noise_v);
            }
            internal::current_calibration_state.progress = 0.3f;

            // Check feedback inversion
            LOG_DEBUG(TAG, "Checking feedback inversion");
            {
                FeedbackInversionParams params;
                params.dc_center = dc_center;
                params.dc_delta = 0.2;
                params.fb_noise = noise_v;
                if (Status err = check_feedback_inversion(params, motor_channel, analog_channel, fb_inverted); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Feedback invertion detection failed with error [%s]", ErrorToString(err));
                    return Status::Unknown;
                }
                LOG_DEBUG(TAG, "==> Feedback inverted : %s", fb_inverted? "true": "false");
            }
            internal::current_calibration_state.progress = 0.4f;

            // Test feedback latency
            LOG_DEBUG(TAG, "Estimating feedback latency");
            {
                FeedbackLatencyParams params;
                params.dc_center = dc_center;
                params.dc_delta = 0.2;
                params.fb_noise = noise_v;
                params.nb_samples = 6;
                params.nb_subsamples = 20;
                if (Status err = get_feedback_latency(params, motor_channel, analog_channel, fb_latency_ms); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Feedback latency estimation failed with error [%s]", ErrorToString(err));
                    return Status::Unknown;
                }
                LOG_DEBUG(TAG, "==> Feedback latency : %.1f ms", fb_latency_ms);
            }
            internal::current_calibration_state.progress = 0.5f;

            // Test deadband size
            LOG_DEBUG(TAG, "Testing deadband size");
            {
                if  (noise_v > 0.01f) // If noise > 10mV, hardcode deadband because we won't be able to detect it properly
                {
                    LOG_WARNING(TAG, "Feedback noise is quite high (%.3f V > 10mV). Hardcoding deadband size of around 1 degree", noise_v);
                    db_size = 0.01; // around 1 degree, which is a common value for servomotors.
                }
                else
                {
                    DeadbandSizeParams params;
                    params.dc_center = dc_center;
                    params.max_dc = 2.0;
                    params.fb_noise = noise_v;
                    if (Status err = get_deadband_size(params, motor_channel, analog_channel, db_size); err != Status::Ok)
                    {
                        // LOG_ERROR(TAG, "Deadband size estimation failed with error [%s]", ErrorToString(err));
                        return Status::Unknown;
                    }
                }
                LOG_DEBUG(TAG, "==> Deadband size : %.2f ms (%.2f deg)", db_size, db_size * 90.f);
            }
            internal::current_calibration_state.progress = 0.6f;

            // Test min bound
            LOG_DEBUG(TAG, "Detecting minimum bound");
            {
                PhysicalBoundParams params;
                params.dc_center = dc_center;
                params.dc_max = 2.9;
                params.dc_min = 0.1;
                params.dc_safe = 0.3;
                params.direction = -1;
                if (Status err = get_physical_bound(params, motor_channel, analog_channel, min_bound); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Minimum bound detection failed with error [%s]", ErrorToString(err));
                    return Status::Unknown;
                }
                LOG_DEBUG(TAG, "==> Minimum bound : %.2f Duty cycle and %.2f V", min_bound.dc_value, min_bound.fb_value);
            }
            internal::current_calibration_state.progress = 0.7f;

            // Test max bound
            LOG_DEBUG(TAG, "Detecting maximum bound");
            {
                PhysicalBoundParams params;
                params.dc_center = dc_center;
                params.dc_max = 2.9;
                params.dc_min = 0.1;
                params.dc_safe = 0.3;
                params.direction = 1;
                if (Status err = get_physical_bound(params, motor_channel, analog_channel, max_bound); err != Status::Ok)
                {
                    // LOG_ERROR(TAG, "Maximum bound detection failed with error [%s]", ErrorToString(err));
                    return Status::Unknown;
                }
                LOG_DEBUG(TAG, "==> Maximum bound : %.2f Duty cycle and %.2f V", max_bound.dc_value, max_bound.fb_value);
            }
            internal::current_calibration_state.progress = 0.8f;

            // Test max speed
            LOG_DEBUG(TAG, "Testing maximum speed");
            {
                // TODO : Implement max speed estimation (by moving between the two bounds and measuring the time)
                LOG_WARNING(TAG, "Maximum speed estimation not implemented yet, defaulting to 6.0 rad/s");
            }
            internal::current_calibration_state.progress = 0.9f;

            LOG_DEBUG(TAG, "Calibration complete, centering motor before disabling it");
            {
                RETURN_ON_ERROR(MotorDriver::SetDutyCycle(motor_channel, dc_center))
                RETURN_ON_ERROR(MotorDriver::SendData());
                vTaskDelay(pdMS_TO_TICKS(800));
                RETURN_ON_ERROR(MotorDriver::DisableAllMotors());
            }

            // Save calibration data
            LOG_DEBUG(TAG, "Exporting calibration data");
            {
                calibration_data.dc_min = min_bound.dc_value;
                calibration_data.dc_max = max_bound.dc_value;
                calibration_data.dc_deadband = db_size;
                calibration_data.fb_min = min_bound.fb_value;
                calibration_data.fb_max = max_bound.fb_value;
                calibration_data.fb_noise = noise_v;
                calibration_data.fb_inverted = fb_inverted;
                calibration_data.fb_latency_ms = fb_latency_ms;
                calibration_data.max_speed = 6.0f; // TODO: implement max speed estimation
            }
            internal::current_calibration_state.progress = 1.0f;

            return Status::Ok;
        }
    }

    
    Status Init(Joint::Id joint)
    {
        if (!internal::initialized)
        {
            // First initialize the NVS handle
            if (Status err = NVS::Init(); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Failed to initialize NVS. Error [%s]", StatusToString(err));
                return err;
            }
            if (Status err = NVS::Open("MtrCalib", &internal::nvshandle_ptr); err != Status::Ok)
            {
                LOG_ERROR(TAG, "Failed to open NVS namespace. Error [%s]", StatusToString(err));
                return err;
            }

            // Set all joints to uncalibrated state
            for (int i = 0; i < (int)Joint::Id::Count; i++)
            {
                internal::calibration_state_registry[i] = JointCalibrationState::UNCALIBRATED;
            }

            internal::initialized = true;
        }

        // Now that we are initialized, load the calibration data for the specified joint
        if (joint >= Joint::Id::Count) return Status::InvalidParameters;

        if (Status err = internal::load_calibration_data(joint); err != Status::Ok)
        {
            return err;
        }

        return Status::Ok;
    }

    Status Deinit()
    {
        if (!internal::initialized) return Status::Ok;

        // Close the NVS handle
        NVS::Close(internal::nvshandle_ptr);

        internal::nvshandle_ptr = nullptr;
        internal::initialized = false;
        return Status::Ok;
    }

    Status StartJointCalibration(Joint::Id joint_id)
    {
        if (!internal::initialized) return Status::InvalidState;
        if (joint_id >= Joint::Id::Count) return Status::InvalidParameters;
        if (internal::calibration_task_handle) return Status::InvalidState; // Calibration already running

        Joint* joint_ptr = Joint::GetJoint(joint_id);
        if (!joint_ptr)
        {
            LOG_ERROR(TAG, "Failed to get joint pointer for joint [%d]. Cannot start calibration.", (int)joint_id);
            return Status::NotFound;
        }
        
        LOG_INFO(TAG, "Starting motor calibration");
        internal::current_calibration_state.running = true;        

        Joint::Id* joint_id_ptr = new Joint::Id(joint_id); // allocate on heap to pass to task
        BaseType_t ret = xTaskCreatePinnedToCore([](void* param) {
            Joint::Id* joint_id_ptr = (Joint::Id*)param;
            Joint::Id joint_id = *joint_id_ptr;
            Joint* joint_ptr = Joint::GetJoint(joint_id);
            if (!joint_ptr)
            {
                LOG_ERROR(TAG, "Failed to get joint pointer for joint [%d]. Cannot start calibration.", (int)joint_id);
                internal::current_calibration_state.running = false;
                return;
            }

            // Stop the control loop and take over for the calibration
            ControlLoop& ctrl = Robot::GetInstance().getControlLoop();
            bool wasRunning = ctrl.isRunning();
            if (wasRunning)
            {
                LOG_DEBUG(TAG, "Disabling control loop");
                if (Status err = ctrl.stop(); err != Status::Ok)
                {
                    LOG_ERROR(TAG, "Failed to stop control loop for motor calibration. Error [%s]", StatusToString(err));
                    internal::current_calibration_state.running = false;
                    return;
                }
            }

            // disable all joints for safety
            for (int i = 0; i < (int)Joint::Id::Count; i++)
            {
                Joint* joint = Joint::GetJoint((Joint::Id)i);
                if (joint) joint->disable();
            }

            MotorCalibration::CalibrationState& calib_state = internal::current_calibration_state;
            calib_state.joint_id = joint_id;
            calib_state.progress = 0.0f;
            calib_state.has_error = false;
            calib_state.error_id = 0;

            // Local copy of the current calibration
            CalibrationData calib_data = internal::calibration_data_registry[(int)joint_id];
            Status err = internal::run_calibration_sequence(
                calib_data,
                (MotorDriver::Channel)joint_ptr->getConfig().motor_driver_index,
                (AnalogDriver::Channel)joint_ptr->getConfig().analog_reader_index
            );

            // Disable motor for safety
            joint_ptr->disable(); // don't care about return value or error here

            if (err != Status::Ok)
            {
                LOG_ERROR(TAG, "Motor calibration failed with error [%s]", StatusToString(err));
                Error::RegisterErrorEvent(ErrorEventMotorCalibrationFailed(joint_ptr->getConfig().motor_driver_index, joint_ptr->getConfig().analog_reader_index));
                calib_state.has_error = true;
                calib_state.error_id = Error::GetErrorEventByIndex(0).eventId;
                internal::calibration_state_registry[(int)joint_id] = JointCalibrationState::ERROR;
            }
            else
            {
                LOG_INFO(TAG, "Motor calibrated, saving calibration data");

                // Save the calibration data to NVS
                internal::calibration_data_registry[(int)joint_id] = calib_data;
                if (Status err = internal::save_calibration_data(joint_id); err != Status::Ok)
                {
                    LOG_ERROR(TAG, "Failed to save calibration data to NVS. Error [%s]", StatusToString(err));
                    internal::calibration_state_registry[(int)joint_id] = JointCalibrationState::ERROR;
                    // TODO : as_error + error event trigger
                }
                else
                {
                    internal::calibration_state_registry[(int)joint_id] = JointCalibrationState::CALIBRATED;
                    LOG_INFO(TAG, "Motor calibration completed successfully");
                }
            }

            // Restart the control loop (back to normal operation)
            if (wasRunning)
            {
                LOG_DEBUG(TAG, "Restarting control loop");
                if (Status err = Robot::GetInstance().getControlLoop().start(); err != Status::Ok)
                {
                    LOG_ERROR(TAG, "Failed to start control loop after motor calibration. Error [%s]", StatusToString(err));
                    internal::current_calibration_state.running = false;
                    return;
                }
            }

            // clean up task
            internal::current_calibration_state.running = false;
            internal::calibration_task_handle = nullptr;
            delete joint_id_ptr; // free the allocated memory for joint_id
            vTaskDelete(nullptr);
        }, "MotorCalib", 4096, joint_id_ptr, tskIDLE_PRIORITY + 1, &internal::calibration_task_handle, CORE_BRAIN);

        if (ret != pdPASS)
        {
            LOG_ERROR(TAG, "Failed to create calibration task");
            internal::current_calibration_state.running = false;
            return Status::NoMemory;
        }

        return Status::Ok;
    }

    Status StopJointCalibration()
    {
        if (!internal::initialized) return Status::InvalidState;
        
        if (!internal::calibration_task_handle)
        {
            LOG_WARNING(TAG, "StopJointCalibration called but no calibration is currently running");
            return Status::Ok;
        }

        vTaskDelete(internal::calibration_task_handle);
        internal::calibration_task_handle = nullptr;

        // Disable the motor for safety
        Joint::Id joint_id = internal::current_calibration_state.joint_id;
        Joint* joint_ptr = Joint::GetJoint(joint_id);
        if (joint_ptr) joint_ptr->disable();

        internal::current_calibration_state.running = false;
        internal::current_calibration_state.progress = 0.0f;
        internal::current_calibration_state.has_error = false;

        return Status::Ok;
    }

    const CalibrationState& GetCalibrationState()
    {
        return internal::current_calibration_state;
    }

    const CalibrationData& GetJointCalibrationData(Joint::Id joint)
    {
        return internal::calibration_data_registry[(int)joint];
    }

    Status SetJointCalibrationData(Joint::Id joint, CalibrationData& data, bool save)
    {
        if (!internal::initialized) return Status::InvalidState;
        if (joint >= Joint::Id::Count) return Status::InvalidParameters;

        internal::calibration_data_registry[(int)joint] = data;
        internal::calibration_state_registry[(int)joint] = JointCalibrationState::CALIBRATED;

        if (save)
        {
            if (Status err = internal::save_calibration_data(joint); err != Status::Ok)
                return err;
        }
        return Status::Ok;
    }

    Status DeleteJointCalibrationData(Joint::Id joint, bool save)
    {
        if (!internal::initialized) return Status::InvalidState;
        if (joint >= Joint::Id::Count) return Status::InvalidParameters;
        
        Joint* joint_ptr = Joint::GetJoint(joint);
        if (!joint_ptr)
        {
            LOG_ERROR(TAG, "Failed to get joint pointer for joint [%d]. Cannot delete calibration data.", (int)joint);
            return Status::NotFound;
        }

        internal::calibration_data_registry[(int)joint] = internal::create_default_calibration(joint_ptr->getConfig());
        internal::calibration_state_registry[(int)joint] = JointCalibrationState::UNCALIBRATED;

        if (save)
        {
            if (Status err = internal::delete_calibration_data(joint); err != Status::Ok)
                return err;
        }
        return Status::Ok;
    }

    JointCalibrationState GetJointCalibrationState(Joint::Id joint)
    {
        return internal::calibration_state_registry[(int)joint];
    }
}