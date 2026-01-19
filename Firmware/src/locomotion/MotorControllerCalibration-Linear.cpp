#include "locomotion/MotorController.hpp"
#include "common/FastRegression.hpp"
#include "common/Log.hpp"
#include <cmath>

constexpr const char* TAG = "MtrCtrl-Calib";

Error MotorController::detect_position_feedback_noise(AnalogDriver::Value* out_noise_level_mV)
{
    constexpr uint8_t CALIB_NOISE_NB_SAMPLES = 20; // Number of samples to gather for noise detection
    constexpr uint16_t CALIB_NOISE_SAMPLE_DELAY_MS = 50; // Delay between noise samples in ms
    
    // Gather samples
    AnalogDriver::Value noise_samples[CALIB_NOISE_NB_SAMPLES];
    for (uint8_t i = 0; i < CALIB_NOISE_NB_SAMPLES; i++)
    {
        if (Error err = AnalogDriver::GetVoltage(analog_channel, &noise_samples[i]); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration");
            calibration_state = UNCALIBRATED;
            return err;
        }
        vTaskDelay(pdMS_TO_TICKS(CALIB_NOISE_SAMPLE_DELAY_MS));
    }

    // Calculate noise (simple min/max difference for now)
    // TODO : improve with standard deviation or other method
    AnalogDriver::Value noise_min = noise_samples[0];
    AnalogDriver::Value noise_max = noise_samples[0];
    for (uint8_t i = 1; i < CALIB_NOISE_NB_SAMPLES; i++)
    {
        if (noise_samples[i] < noise_min) noise_min = noise_samples[i];
        if (noise_samples[i] > noise_max) noise_max = noise_samples[i];
    }
    *out_noise_level_mV = noise_max - noise_min;

    return Error::None;
}

void MotorController::abort_calibration()
{
    // resetting calibration data (try to load existing data from NVS, otherwise use defaults)
    Log::Add(Log::Level::Info, TAG, "Resetting calibration.");
    if (nvshandle_ptr->get("calib_data", calibration_data) != Error::None)
    {
        calibration_data = DEFAULT_CALIBRATION;
    }

    // disabling motor
    Log::Add(Log::Level::Info, TAG, "Disabling motor.");
    MotorDriver::SetPWM(motor_channel, 0);

    // Mark as uncalibrated
    calibration_state = UNCALIBRATED;
}


Error MotorController::detect_servo_pwm_deadband(MotorDriver::Value default_position, AnalogDriver::Value feedback_noise_level_mV, MotorDriver::Value* out_deadband_pwm)
{
    constexpr MotorDriver::Value CALIB_DEADBAND_TEST_PWM_INCREMENT = 1; // PWM increment for deadband detection
    constexpr uint16_t CALIB_DEADBAND_TEST_DELAY_MS = 200; // Delay after changing PWM to allow motor to move
    constexpr MotorDriver::Value CALIB_DEADBAND_MAX_TEST_RANGE = 50; // Maximum PWM range to test for deadband    
    constexpr uint8_t CALIB_DEADBAND_SAMPLES = 10; // Number of samples to determine deadband

    // Set servo to default position 
    if (Error err = MotorDriver::SetPWM(motor_channel, default_position); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration");
        calibration_state = UNCALIBRATED;
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // wait for motor to settle

    // Measure default position voltage
    AnalogDriver::Value default_voltage = 0;
    if (Error err = AnalogDriver::GetVoltage(analog_channel, &default_voltage); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration");
        calibration_state = UNCALIBRATED;
        return err;
    }

    MotorDriver::Value current_pwm = default_position;
    AnalogDriver::Value current_voltage = default_voltage;
    MotorDriver::Value pwm_deadbands[CALIB_DEADBAND_SAMPLES];
    for (uint8_t sample_idx = 0; sample_idx < CALIB_DEADBAND_SAMPLES; sample_idx++)
    {       
        MotorDriver::Value base_pwm = current_pwm;
        AnalogDriver::Value base_voltage = current_voltage;
        while (std::abs(current_voltage - base_voltage) < feedback_noise_level_mV)
        {
            current_pwm += CALIB_DEADBAND_TEST_PWM_INCREMENT;
            if (Error err = MotorDriver::SetPWM(motor_channel, current_pwm); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration");
                calibration_state = UNCALIBRATED;
                return err;
            }
            vTaskDelay(pdMS_TO_TICKS(CALIB_DEADBAND_TEST_DELAY_MS));
            if (Error err = AnalogDriver::GetVoltage(analog_channel, &current_voltage); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration");
                calibration_state = UNCALIBRATED;
                return err;
            }
            if (current_pwm - base_pwm > CALIB_DEADBAND_MAX_TEST_RANGE)
            {
                Log::Add(Log::Level::Warning, TAG, "Failed to detect deadband within maximum test range");
                *out_deadband_pwm = 0;
                return Error::HardwareFailure;
            }
        }
        MotorDriver::Value deadband = current_pwm - base_pwm;
        pwm_deadbands[sample_idx] = deadband;
    }

    // Use the maximum deadband from samples
    int deadband_max = 0;
    for (uint8_t i = 0; i < CALIB_DEADBAND_SAMPLES; i++)
    {
        if (pwm_deadbands[i] > deadband_max)
        {
            deadband_max = pwm_deadbands[i];
        }
    }
    *out_deadband_pwm = static_cast<MotorDriver::Value>(deadband_max);

    return Error::None;
}


Error MotorController::detect_feedback_latency(AnalogDriver::Value noise_level_mV, TickType_t* out_latency_ticks)
{
    constexpr int CALIB_LATENCY_NB_SAMPLES = 10; // Number of samples to gather for latency detection
    constexpr MotorDriver::Value CALIB_LATENCY_PWM_OFFSET = 20; // PWM offset to apply when moving motor for latency detection
    constexpr TickType_t CALIB_LATENCY_MAX_WAIT_TICKS = pdMS_TO_TICKS(2000); // Maximum wait time for voltage change
    TickType_t latency_samples[CALIB_LATENCY_NB_SAMPLES];

    for (int i = 0; i < CALIB_LATENCY_NB_SAMPLES; i++)
    {
        // set the motor to center (1.5ms at 50Hz)
        MotorDriver::Value center_pwm = static_cast<MotorDriver::Value>( (4096 * 1.5f) / 20 );
        if (Error err = MotorDriver::SetPWM(motor_channel, center_pwm); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration");
            return err;
        }

        // wait a bit for motor to settle (should already be around center, but just in case)
        vTaskDelay(pdMS_TO_TICKS(500));

        // get the voltage
        AnalogDriver::Value base_voltage = 0;
        if (Error err = AnalogDriver::GetVoltage(analog_channel, &base_voltage); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration");
            return err;
        }

        // move the motor
        MotorDriver::Value test_pwm = center_pwm + CALIB_LATENCY_PWM_OFFSET;
        if (Error err = MotorDriver::SetPWM(motor_channel, test_pwm); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration");
            return err;
        }

        // measure time until voltage changes beyond noise level
        TickType_t start_tick = xTaskGetTickCount();
        TickType_t current_tick = start_tick;
        AnalogDriver::Value current_voltage = base_voltage;
        bool has_moved = false;
        while (!has_moved)
        {
            if (Error err = AnalogDriver::GetVoltage(analog_channel, &current_voltage); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration");
                return err;
            }

            if (std::abs(current_voltage - base_voltage) > noise_level_mV)
            {
                // voltage changed beyond noise level, record latency
                latency_samples[i] = current_tick - start_tick;
                has_moved = true;
            }

            vTaskDelay(pdMS_TO_TICKS(5)); // small delay before next check (smaller than control loop frequency)
            current_tick = xTaskGetTickCount();

            if ((current_tick - start_tick) > CALIB_LATENCY_MAX_WAIT_TICKS)
            {
                Log::Add(Log::Level::Error, TAG, "Timeout waiting for voltage change during latency detection");
                return Error::HardwareFailure;
            }
        }

        // store latency sample
        latency_samples[i] = current_tick - start_tick;
    }

    // calculate average latency
    TickType_t latency_sum = 0;
    for (int i = 0; i < CALIB_LATENCY_NB_SAMPLES; i++)
    {
        latency_sum += latency_samples[i];
    }
    *out_latency_ticks = latency_sum / CALIB_LATENCY_NB_SAMPLES;
    return Error::None;
}


Error MotorController::run_calibration_sequence()
{
    constexpr uint16_t CALIB_SAFEGUARD_MIN_PWM = 60; // absolute minimum PWM to avoid breaking the motor
    constexpr uint16_t CALIB_SAFEGUARD_MAX_PWM = 570; // absolute maximum PWM to avoid breaking the motor
    constexpr AnalogDriver::Value CALIB_FEEDBACK_NOISE_ERR_THRESHOLD_MV = 20; // Maximum acceptable noise level in mV
    constexpr AnalogDriver::Value CALIB_FEEDBACK_NOISE_MIN_MV = 4; // Minimum detectable noise level in mV
    constexpr MotorDriver::Value CALIB_DEADBAND_ERR_THRESHOLD_PWM_MAX = 20; // Maximum acceptable deadband in PWM units
    constexpr MotorDriver::Value CALIB_DEADBAND_MIN_PWM = 2; // Minimum deadband in PWM units

    Log::Add(Log::Level::Info, TAG, "Motor calibration sequence started");
    calibration_state = CALIBRATING;


    ///=== Moving motor to center position ===///
    // NOTE : assuming duty cycle of 1.5ms at 50Hz
    Log::Add(Log::Level::Info, TAG, "Moving motor to center position ...");
    MotorDriver::Value center_pwm = static_cast<MotorDriver::Value>( (4096 * 1.5f) / 20 );
    if (Error err = MotorDriver::SetPWM(motor_channel, center_pwm); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }


    ///=== Wait 1 second for motor to settle ===///
    vTaskDelay(pdMS_TO_TICKS(1000));


    ///=== Detect position feedback noise level ===///
    Log::Add(Log::Level::Info, TAG, "Detecting position feedback noise level ...");
    AnalogDriver::Value feedback_noise;
    if (Error err = detect_position_feedback_noise(&feedback_noise); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to detect position feedback noise, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }
    if (feedback_noise > CALIB_FEEDBACK_NOISE_ERR_THRESHOLD_MV)
    {
        Log::Add(Log::Level::Error, TAG, "Position feedback noise too high (%d mV), aborting calibration.", feedback_noise);
        abort_calibration();
        return Error::HardwareFailure;
    }
    if (feedback_noise < CALIB_FEEDBACK_NOISE_MIN_MV)
    {
        Log::Add(Log::Level::Warning, TAG, "Position feedback noise seems too low (%d mV), setting to minimum detectable level.", feedback_noise);
        feedback_noise = CALIB_FEEDBACK_NOISE_MIN_MV;
    }
    Log::Add(Log::Level::Info, TAG, "Position feedback noise level: %d mV", feedback_noise);
    calibration_data.feedback_noise = feedback_noise;


    ///=== Detect servo pwm deadband ===///
    Log::Add(Log::Level::Info, TAG, "Detecting servo PWM deadband ...");
    MotorDriver::Value servo_deadband;
    if (Error err = detect_servo_pwm_deadband(center_pwm, feedback_noise, &servo_deadband); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to detect servo PWM deadband, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        calibration_state = UNCALIBRATED;
        return err;
    }
    if (servo_deadband > CALIB_DEADBAND_ERR_THRESHOLD_PWM_MAX)
    {
        Log::Add(Log::Level::Error, TAG, "Detected servo PWM deadband too large (%d), aborting calibration.", servo_deadband);
        calibration_state = UNCALIBRATED;
        return Error::HardwareFailure;
    }
    if(servo_deadband < CALIB_DEADBAND_MIN_PWM)
    {
        Log::Add(Log::Level::Warning, TAG, "Detected servo PWM deadband seems too small (%d), setting to minimum threshold.", servo_deadband);
        servo_deadband = CALIB_DEADBAND_MIN_PWM;
    }
    Log::Add(Log::Level::Info, TAG, "Servo PWM deadband: %d", servo_deadband);
    calibration_data.pwm_deadband = servo_deadband;


    ///=== Detect feedback latency ===///
    Log::Add(Log::Level::Info, TAG, "Detecting position feedback latency ...");
    TickType_t feedback_latency_ticks = 0;
    if (Error err = detect_feedback_latency(feedback_noise, &feedback_latency_ticks); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to detect position feedback latency, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }
    Log::Add(Log::Level::Info, TAG, "Position feedback latency: %d ticks (around %d ms)", feedback_latency_ticks, pdTICKS_TO_MS(feedback_latency_ticks));
    calibration_data.feedback_latency_ms = pdTICKS_TO_MS(feedback_latency_ticks);


    ///=== Moving motor back to center ===///
    Log::Add(Log::Level::Info, TAG, "Moving motor back to center position ...");
    if (Error err = MotorDriver::SetPWM(motor_channel, center_pwm); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    constexpr MotorDriver::Value CALIB_MOVE_INCREMENT_PWM = 1; // PWM increment when moving
    constexpr MotorDriver::Value CALIB_LOOP_DELAY_MS = 20; // Delay between each loop iteration
    constexpr MotorDriver::Value CALIB_BASELINE_MAX_PWM = 60; // Maximum PWM shift from center to establish baseline
    constexpr float CALIB_VOLTAGE_ERROR_THRESHOLD_STD_MULTIPLIER = 4.5f; // Multiplier for standard deviation to determine voltage error threshold
    constexpr int CALIB_NB_STALLS_TO_CONFIRM = 3; // Number of consecutive stall detections to confirm stall
    int FEEDBACK_PWM_SHIFT = std::round((float)calibration_data.feedback_latency_ms / (CALIB_LOOP_DELAY_MS)); // number of samples to shift based on feedback latency
    Log::Add(Log::Level::Info, TAG, "FEEDBACK_PWM_SHIFT = %d samples", FEEDBACK_PWM_SHIFT);

    // TODO : We could improve the stall windback by going back to where the voltage gap started to increase
    // this would require storing previous voltage readings, but it would go back to the stall's beginning instead of the stall detection's begining.

    ///=== FIND MIN POSITION STALL ===///
    {
        bool has_stalled = false;
        MotorDriver::Value current_pwm = center_pwm;
        AnalogDriver::Value current_voltage = 0;
        int iteration_counter = 0;
        int nb_consecutive_stall_detections = 0;
        MotorDriver::Value expected_pwm;

        FastRegression baseline_regression;
        float slope = 0.0f;
        float offset = 0.0f;
        float error_std = 0.0f;

        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t xFrequency = pdMS_TO_TICKS(CALIB_LOOP_DELAY_MS);

        Log::Add(Log::Level::Info, TAG, "Starting motor movement to detect maximum stall ...");
        while (!has_stalled)
        {
            current_pwm += CALIB_MOVE_INCREMENT_PWM;
            if (current_pwm > CALIB_SAFEGUARD_MAX_PWM)
            {
                Log::Add(Log::Level::Warning, TAG, "Reached maximum safeguard PWM limit during calibration");
                abort_calibration();
                return Error::HardwareFailure;
            }

            if (Error err = MotorDriver::SetPWM(motor_channel, current_pwm); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration. Error: %d", static_cast<uint8_t>(err));
                abort_calibration();
                return err;
            }
            
            if (Error err = AnalogDriver::GetVoltage(analog_channel, &current_voltage); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration. Error: %d", static_cast<uint8_t>(err));
                abort_calibration();
                return err;
            }

            if (iteration_counter < (CALIB_BASELINE_MAX_PWM / CALIB_MOVE_INCREMENT_PWM))
            {
                if (iteration_counter >= FEEDBACK_PWM_SHIFT)
                {
                    // We are in valid conditions to add a point to the baseline estimation
                    // Here, the voltage corresponds to the pwm sent FEEDBACK_PWM_SHIFT iterations ago
                    // So the corresponding pwm is :
                    MotorDriver::Value baseline_pwm = current_pwm - (FEEDBACK_PWM_SHIFT * CALIB_MOVE_INCREMENT_PWM);
                    AnalogDriver::Value baseline_voltage = current_voltage;
                    // Those are our x and y coordinates for the baseline line,
                    // We can add them to the regression object
                    baseline_regression.addPoint(baseline_pwm, baseline_voltage);
                }
            }
            else
            {
                // We are past the baseline collection phase
                // We can now estimate the expected voltage based on the baseline regression

                // First check if we calculated the regression parameters, if not do it now
                if (slope == 0.0f && offset == 0.0f)
                {
                    Log::Add(Log::Level::Info, TAG, "Computing baseline regression ...");

                    if (!baseline_regression.compute(slope, offset, error_std))
                    {
                        Log::Add(Log::Level::Error, TAG, "Failed to compute baseline regression during calibration.");
                        abort_calibration();
                        return Error::SoftwareFailure;
                    }

                    if (error_std < calibration_data.feedback_noise)
                    {
                        Log::Add(Log::Level::Warning, TAG, "Baseline regression error std (%.2f) is less than feedback noise (%d mV), adjusting.", error_std, calibration_data.feedback_noise);
                        error_std = (float)calibration_data.feedback_noise; // ensure error std is at least equal to feedback noise
                    }

                    Log::Add(Log::Level::Info, TAG, "Baseline regression computed: slope=%.4f, offset=%.2f, error_std=%.2f", slope, offset, error_std);
                }

                // Ok, now we can estimate the expected voltage.

                // First, determine the pwm that corresponds to the current voltage reading
                expected_pwm = current_pwm - (FEEDBACK_PWM_SHIFT * CALIB_MOVE_INCREMENT_PWM);
                // Then, estimate the expected voltage from the regression line
                float expected_voltage = (slope * expected_pwm) + offset;
                // Now, compare with actual voltage
                float voltage_error = std::abs((float)current_voltage - expected_voltage);
                // Log::Add(Log::Level::Debug, TAG, "PWM=%d, Measured Voltage=%d mV, Expected Voltage=%.2f mV, Error=%.2f mV", expected_pwm, current_voltage, expected_voltage, voltage_error);
                // If the error exceeds threshold, we have stalled
                float error_threshold = CALIB_VOLTAGE_ERROR_THRESHOLD_STD_MULTIPLIER * error_std;
                if (voltage_error > error_threshold)
                {
                    nb_consecutive_stall_detections++;
                    Log::Add(Log::Level::Debug, TAG, "Stall detection %d/%d", nb_consecutive_stall_detections, CALIB_NB_STALLS_TO_CONFIRM);

                    if (nb_consecutive_stall_detections >= CALIB_NB_STALLS_TO_CONFIRM)
                    {
                        has_stalled = true;
                        Log::Add(Log::Level::Info, TAG, "Motor stall detected at PWM=%d, Voltage= %d mV (expected %.2f mV, error %.2f mV > threshold %.2f mV)", expected_pwm, current_voltage, expected_voltage, voltage_error, error_threshold);
                    }
                }
                else
                {
                    nb_consecutive_stall_detections = 0; // reset counter
                }
            }

            iteration_counter++;
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        }

        // wind-back at the beginning of detection for better accuracy
        expected_pwm -= ((CALIB_NB_STALLS_TO_CONFIRM-1) * CALIB_MOVE_INCREMENT_PWM);
        current_voltage -= ((CALIB_NB_STALLS_TO_CONFIRM-1) * CALIB_MOVE_INCREMENT_PWM) * slope; // approximate voltage after wind-back

        Log::Add(Log::Level::Info, TAG, "Stall position determined at PWM=%d, Voltage= %d mV (after windback)", expected_pwm, current_voltage);

        calibration_data.max_pwm = expected_pwm;
        calibration_data.max_voltage = current_voltage;
    }

    ///=== Moving motor back to center ===///
    Log::Add(Log::Level::Info, TAG, "Moving motor back to center position ...");
    if (Error err = MotorDriver::SetPWM(motor_channel, center_pwm); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(500));


    ///=== FIND MIN POSITION STALL ===///
    {
        bool has_stalled = false;
        MotorDriver::Value current_pwm = center_pwm;
        AnalogDriver::Value current_voltage = 0;
        int iteration_counter = 0;
        int nb_consecutive_stall_detections = 0;
        MotorDriver::Value expected_pwm;

        FastRegression baseline_regression;
        float slope = 0.0f;
        float offset = 0.0f;
        float error_std = 0.0f;

        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t xFrequency = pdMS_TO_TICKS(CALIB_LOOP_DELAY_MS);

        Log::Add(Log::Level::Info, TAG, "Starting motor movement to detect minimum stall ...");
        while (!has_stalled)
        {
            current_pwm -= CALIB_MOVE_INCREMENT_PWM;
            if (current_pwm < CALIB_SAFEGUARD_MIN_PWM)
            {
                Log::Add(Log::Level::Warning, TAG, "Reached minimum safeguard PWM limit during calibration");
                abort_calibration();
                return Error::HardwareFailure;
            }

            if (Error err = MotorDriver::SetPWM(motor_channel, current_pwm); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration. Error: %d", static_cast<uint8_t>(err));
                abort_calibration();
                return err;
            }
            
            if (Error err = AnalogDriver::GetVoltage(analog_channel, &current_voltage); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration. Error: %d", static_cast<uint8_t>(err));
                abort_calibration();
                return err;
            }

            if (iteration_counter < (CALIB_BASELINE_MAX_PWM / CALIB_MOVE_INCREMENT_PWM))
            {
                if (iteration_counter >= FEEDBACK_PWM_SHIFT)
                {
                    // We are in valid conditions to add a point to the baseline estimation
                    // Here, the voltage corresponds to the pwm sent FEEDBACK_PWM_SHIFT iterations ago
                    // So the corresponding pwm is :
                    MotorDriver::Value baseline_pwm = current_pwm - (FEEDBACK_PWM_SHIFT * CALIB_MOVE_INCREMENT_PWM);
                    AnalogDriver::Value baseline_voltage = current_voltage;
                    // Those are our x and y coordinates for the baseline line,
                    // We can add them to the regression object
                    baseline_regression.addPoint(baseline_pwm, baseline_voltage);
                }
            }
            else
            {
                // We are past the baseline collection phase
                // We can now estimate the expected voltage based on the baseline regression

                // First check if we calculated the regression parameters, if not do it now
                if (slope == 0.0f && offset == 0.0f)
                {
                    Log::Add(Log::Level::Info, TAG, "Computing baseline regression ...");

                    if (!baseline_regression.compute(slope, offset, error_std))
                    {
                        Log::Add(Log::Level::Error, TAG, "Failed to compute baseline regression during calibration.");
                        abort_calibration();
                        return Error::SoftwareFailure;
                    }

                    if (error_std < calibration_data.feedback_noise)
                    {
                        Log::Add(Log::Level::Warning, TAG, "Baseline regression error std (%.2f) is less than feedback noise (%d mV), adjusting.", error_std, calibration_data.feedback_noise);
                        error_std = (float)calibration_data.feedback_noise; // ensure error std is at least equal to feedback noise
                    }

                    Log::Add(Log::Level::Info, TAG, "Baseline regression computed: slope=%.4f, offset=%.2f, error_std=%.2f", slope, offset, error_std);
                }

                // Ok, now we can estimate the expected voltage.

                // First, determine the pwm that corresponds to the current voltage reading
                expected_pwm = current_pwm - (FEEDBACK_PWM_SHIFT * CALIB_MOVE_INCREMENT_PWM);
                // Then, estimate the expected voltage from the regression line
                float expected_voltage = (slope * expected_pwm) + offset;
                // Now, compare with actual voltage
                float voltage_error = std::abs((float)current_voltage - expected_voltage);
                // Log::Add(Log::Level::Debug, TAG, "PWM=%d, Measured Voltage=%d mV, Expected Voltage=%.2f mV, Error=%.2f mV", expected_pwm, current_voltage, expected_voltage, voltage_error);
                // If the error exceeds threshold, we have stalled
                float error_threshold = CALIB_VOLTAGE_ERROR_THRESHOLD_STD_MULTIPLIER * error_std;
                if (voltage_error > error_threshold)
                {
                    nb_consecutive_stall_detections++;
                    Log::Add(Log::Level::Debug, TAG, "Stall detection %d/%d", nb_consecutive_stall_detections, CALIB_NB_STALLS_TO_CONFIRM);

                    if (nb_consecutive_stall_detections >= CALIB_NB_STALLS_TO_CONFIRM)
                    {
                        has_stalled = true;
                        Log::Add(Log::Level::Info, TAG, "Motor stall detected at PWM=%d, Voltage= %d mV (expected %.2f mV, error %.2f mV > threshold %.2f mV)", expected_pwm, current_voltage, expected_voltage, voltage_error, error_threshold);
                    }
                }
                else
                {
                    nb_consecutive_stall_detections = 0; // reset counter
                }
            }

            iteration_counter++;
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        }

        // wind-back at the beginning of detection for better accuracy
        expected_pwm += ((CALIB_NB_STALLS_TO_CONFIRM-1) * CALIB_MOVE_INCREMENT_PWM);
        current_voltage += ((CALIB_NB_STALLS_TO_CONFIRM-1) * CALIB_MOVE_INCREMENT_PWM) * slope; // approximate voltage after wind-back

        Log::Add(Log::Level::Info, TAG, "Stall position determined at PWM=%d, Voltage= %d mV (after windback)", expected_pwm, current_voltage);

        calibration_data.min_pwm = expected_pwm;
        calibration_data.min_voltage = current_voltage;
    }

    Log::Add(Log::Level::Info, TAG, "First pass calibration complete. Starting second pass ...");

    // Second pass to be more precise,
    // This time we don't use fancy regression or stall detection.
    // We just move to the bounds, here the voltage should be really close to the stall voltage.
    // And we progressively move back until we detect a voltage change beyond a threshold.
    // This will mean we are out of the stall, and we can record that position as the min/max position.

    constexpr float CALIB_REFINEMENT_NOISE_THRESHOLD_STD_MULTIPLIER = 0.5f; // Multiplier for standard deviation to determine voltage change threshold during refinement
    constexpr MotorDriver::Value CALIB_REFINEMENT_INCREMENT_PWM = 2; // PWM increment for refinement
    constexpr int CALIB_REFINEMENT_NB_SAMPLES = 5; // Number of samples to average during refinement

    /// === MIN POSITION REFINEMENT ===///
    Log::Add(Log::Level::Info, TAG, "Starting min position refinement ...");
    {
        // First, go at the minimum and wait a bit for the motor to settle
        if (Error err = MotorDriver::SetPWM(motor_channel, calibration_data.min_pwm); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration. Error: %d", static_cast<uint8_t>(err));
            abort_calibration();
            return err;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Now we should be at the minimum.
        // Start the main loop, looking for the voltage and going forward until it changes.

        MotorDriver::Value current_pwm = calibration_data.min_pwm;
        AnalogDriver::Value current_voltage = calibration_data.min_voltage;
        while (current_voltage < calibration_data.min_voltage + (CALIB_REFINEMENT_NOISE_THRESHOLD_STD_MULTIPLIER * feedback_noise))
        {
            current_pwm += CALIB_REFINEMENT_INCREMENT_PWM;
            if (Error err = MotorDriver::SetPWM(motor_channel, current_pwm); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration. Error: %d", static_cast<uint8_t>(err));
                abort_calibration();
                return err;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            AnalogDriver::Value voltage_sum = 0;
            for (int i = 0; i < CALIB_REFINEMENT_NB_SAMPLES; i++)
            {
                // take multiple readings to ensure stability
                vTaskDelay(pdMS_TO_TICKS(200));
                if (Error err = AnalogDriver::GetVoltage(analog_channel, &current_voltage); err != Error::None)
                {
                    Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration. Error: %d", static_cast<uint8_t>(err));
                    abort_calibration();
                    return err;
                }
                voltage_sum += current_voltage;
            }
            current_voltage = voltage_sum / CALIB_REFINEMENT_NB_SAMPLES;
            Log::Add(Log::Level::Debug, TAG, "Refinement loop: PWM=%d, Voltage=%d mV, calibVoltage=%d mV, Threshold=%d mV", current_pwm, current_voltage, calibration_data.min_voltage, static_cast<AnalogDriver::Value>(CALIB_REFINEMENT_NOISE_THRESHOLD_STD_MULTIPLIER * feedback_noise));
        }

        Log::Add(Log::Level::Info, TAG, "Min position refinement complete at PWM=%d, Voltage=%d mV instead of base detection at PWM=%d, Voltage=%d mV", current_pwm, current_voltage, calibration_data.min_pwm, calibration_data.min_voltage);
        // refinement goes a little too far
        calibration_data.min_pwm = calibration_data.min_pwm*0.25 + current_pwm*0.75;
        calibration_data.min_voltage = calibration_data.min_voltage*0.25 + current_voltage*0.75;
    }


    /// === MAX POSITION REFINEMENT ===///
    Log::Add(Log::Level::Info, TAG, "Starting max position refinement ...");
    {
        // First, go at the maximum and wait a bit for the motor to settle
        if (Error err = MotorDriver::SetPWM(motor_channel, calibration_data.max_pwm); err != Error::None)
        {
            Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration. Error: %d", static_cast<uint8_t>(err));
            abort_calibration();
            return err;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Now we should be at the maximum.
        // Start the main loop, looking for the voltage and going backward until it changes.

        MotorDriver::Value current_pwm = calibration_data.max_pwm;
        AnalogDriver::Value current_voltage = calibration_data.max_voltage;
        while (current_voltage > calibration_data.max_voltage - (CALIB_REFINEMENT_NOISE_THRESHOLD_STD_MULTIPLIER * feedback_noise))
        {
            current_pwm -= CALIB_REFINEMENT_INCREMENT_PWM;
            if (Error err = MotorDriver::SetPWM(motor_channel, current_pwm); err != Error::None)
            {
                Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM during calibration. Error: %d", static_cast<uint8_t>(err));
                abort_calibration();
                return err;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            AnalogDriver::Value voltage_sum = 0;
            for (int i = 0; i < CALIB_REFINEMENT_NB_SAMPLES; i++)
            {
                // take multiple readings to ensure stability
                vTaskDelay(pdMS_TO_TICKS(200));
                if (Error err = AnalogDriver::GetVoltage(analog_channel, &current_voltage); err != Error::None)
                {
                    Log::Add(Log::Level::Error, TAG, "Failed to read analog voltage during calibration. Error: %d", static_cast<uint8_t>(err));
                    abort_calibration();
                    return err;
                }
                voltage_sum += current_voltage;
            }
            current_voltage = voltage_sum / CALIB_REFINEMENT_NB_SAMPLES;
            Log::Add(Log::Level::Debug, TAG, "Refinement loop: PWM=%d, Voltage=%d mV, calibVoltage=%d mV, Threshold=%d mV", current_pwm, current_voltage, calibration_data.max_voltage, static_cast<AnalogDriver::Value>(CALIB_REFINEMENT_NOISE_THRESHOLD_STD_MULTIPLIER * feedback_noise));
        }

        Log::Add(Log::Level::Info, TAG, "Max position refinement complete at PWM=%d, Voltage=%d mV instead of base detection at PWM=%d, Voltage=%d mV", current_pwm, current_voltage, calibration_data.max_pwm, calibration_data.max_voltage);
        // refinement goes a little too far
        calibration_data.max_pwm = calibration_data.max_pwm*0.25 + current_pwm*0.75;
        calibration_data.max_voltage = calibration_data.max_voltage*0.25 + current_voltage*0.75;
    }

    // Go back to center position
    Log::Add(Log::Level::Info, TAG, "Moving motor back to center position ...");
    if (Error err = MotorDriver::SetPWM(motor_channel, center_pwm); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to set motor PWM, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    // Disable motor
    Log::Add(Log::Level::Info, TAG, "Disabling motor ...");
    if (Error err = MotorDriver::SetPWM(motor_channel, 0); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to disable motor, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }

    // Save calibration data to NVS
    Log::Add(Log::Level::Info, TAG, "Saving calibration data to NVS ...");
    if (Error err = nvshandle_ptr->set("calib_data", calibration_data); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to save calibration data to NVS, aborting calibration. Error: %d", static_cast<uint8_t>(err));
        abort_calibration();
        return err;
    }
    Log::Add(Log::Level::Info, TAG, "Saved calibration data");

    calibration_state = CALIBRATED;
    return Error::None;
}