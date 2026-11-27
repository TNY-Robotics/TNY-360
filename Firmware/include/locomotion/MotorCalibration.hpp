#pragma once
#include "common/utils.hpp"
#include "common/Error.hpp"
#include "locomotion/Joint.hpp"

namespace MotorCalibration
{
    struct CalibrationState
    {
        Joint::Id joint_id = Joint::Id::Count; // The joint being calibrated
        float progress = 0.0f;                 // Progress of the calibration (0.0 to 1.0)
        bool has_error;                        // Flag indicating if an error occurred
        uint16_t error_id;                     // Error event id, only valid if an error occurred
        bool running;                          // Flag indicating if a calibration is currently running
    };

    enum class JointCalibrationState : uint8_t
    {
        UNCALIBRATED = 0,
        CALIBRATING = 1,
        CALIBRATED = 2,
        ERROR = 3,
    };

    /** <API_REF>
     * @type MotorCalibration::CalibrationData
     * @desc Calibration data of a motor.
     * @field dc_min float32 Minimum Duty Cycle value corresponding to the minimum physical position of the motor.
     * @field dc_max float32 Maximum Duty Cycle value corresponding to the maximum physical position of the motor.
     * @field dc_deadband float32 Deadband width in Duty Cycle unit.
     * @field fb_min float32 Feedback value corresponding to the minimum physical position of the motor.
     * @field fb_max float32 Feedback value corresponding to the maximum physical position of the motor.
     * @field fb_noise float32 Feedback noise level in Volt (standard deviation).
     * @field fb_latency_ms float32 Latency between command and movement in ms.
     * @field fb_inverted bool Flag indicating whether the feedback is inverted or not.
     * @field max_speed float32 Maximum speed of the motor in range/s (range = [max bound - min bound]).
     */
    struct CalibrationData
    {
        /// @brief Minimum Duty Cycle value corresponding to the minimum physical position of the motor
        MotorDriver::Value dc_min = 0.5f;
        /// @brief Maximum Duty Cycle value corresponding to the maximum physical position of the motor
        MotorDriver::Value dc_max = 2.5f;
        /// @brief Deadband width in Duty Cycle unit
        MotorDriver::Value dc_deadband = 0;
        /// @brief Feedback value corresponding to the minimum physical position of the motor
        AnalogDriver::Value fb_min = 0.5f;
        /// @brief Feedback value corresponding to the maximum physical position of the motor
        AnalogDriver::Value fb_max = 2.8f;
        /// @brief Feedback noise level in Volt (standard deviation)
        AnalogDriver::Value fb_noise = 0;
        /// @brief Latency between command and movement in ms
        float fb_latency_ms = 0;
        /// @brief Flag indicating whether the feedback is inverted (low voltage for max position, high voltage for min position)
        bool fb_inverted = false;
        /// @brief Maximum speed of the motor (in progress per second, where progress is the distance between min and max bound)
        float max_speed = 0.f;
    };

    /**
     * @brief Initialize the MotorCalibration module (if already initialized, will only load joint calibration).
     * @param joint The joint to initialize the calibration for.
     * @return Error code indicating success or failure.
     */
    Status Init(Joint::Id joint);

    /**
     * @brief Deinitialize the MotorCalibration module.
     * @return Error code indicating success or failure.
     */
    Status Deinit();

    /**
     * @brief Start the calibration process for a specific joint.
     * @param joint The joint to calibrate.
     * @return Error code indicating success or failure.
     * @note This function will start the calibration process on a separate task and will return immediately.
     * @note Use `GetJointCalibrationState` to see calibration progress and state.
     */
    Status StartJointCalibration(Joint::Id joint);

    /**
     * @brief Stop the calibration process if running.
     * @return Error code indicating success or failure.
     * @note This function will stop the calibration process and reset the calibration data for the current joint.
     */
    Status StopJointCalibration();

    /**
     * @brief Get the current calibration process state.
     * @return Calibration state (target joint, progress, etc.)
     */
    const CalibrationState& GetCalibrationState();

    /**
     * @brief Get a joint's calibration data
     * @param joint The joint to get the calibration data for.
     * @return The calibration data of the joint.
     */
    const CalibrationData& GetJointCalibrationData(Joint::Id joint);

    /**
     * @brief Set the calibration data for a specific joint.
     * @param joint The joint to set the calibration data for.
     * @param data The CalibrationData object to set
     * @param save Should the data be saved in NVS for future use
     * @return Error code indicating success or failure.
     */
    Status SetJointCalibrationData(Joint::Id joint, CalibrationData& data, bool save = true);

    /**
     * @brief Delete the calibration data for a specific joint.
     * @param joint The joint to delete the calibration data for.
     * @param save Should the deletion be saved in NVS or not
     * @return Error code indicating success or failure.
     */
    Status DeleteJointCalibrationData(Joint::Id joint, bool save = true);

    /**
     * @brief Get a joint's calibration state
     * @param joint The joint to get the calibration state for.
     * @return The calibration state of the joint.
     */
    JointCalibrationState GetJointCalibrationState(Joint::Id joint);
}
