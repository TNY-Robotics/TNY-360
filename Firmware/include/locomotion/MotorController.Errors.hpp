#pragma once
#include "common/Error.hpp"

enum class SubmoduleID : uint8_t
{
    Initialization = 0x01,
    Calibration = 0x02,
};

enum class CodeID : uint8_t
{
    // Initialization
    
    // Calibration
    FeedbackInversion = 0x01,
    DeadbandSizeEstimation = 0x02,
    LatencyTimeout = 0x03,
    PhysicalBoundNotFound = 0x04,
    CalibrationFailed = 0x05,
};

/** <API_REF>
 * @error 0x08020101 FeedbackInversion
 * @module MotorController
 * @submodule Calibration
 * @severity Trace
 * @desc Failed to detect if a motor feedback is inverted or not.
 * @fix Check motor and analog reader wiring and ensure that everything is properly connected.
 * @payload diff float The difference between expected and actual feedback values.
 * @payload noise float The noise level in the feedback signal.
 */
class ErrorEventFeedbackInversionFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventFeedbackInversionFailed(float diff, float noise) :
        ErrorEventBuilder(Error::ModuleID::MotorController, SubmoduleID::Calibration, CodeID::FeedbackInversion, Error::ErrorSeverity::Trace)
    {
        appendPayload(diff);
        appendPayload(noise);
    }
};

/** <API_REF>
 * @error 0x08020201 DeadbandSizeEstimation
 * @desc Failed to estimate the deadband size of a motor.
 * @fix Check motor, analog reader, and motor driver wiring and ensure that everything is properly connected.
 * @payload collected uint16 The number of valid samples collected during the estimation process.
 * @payload expected uint16 The expected number of samples to be collected for a reliable estimation.
 */
class ErrorEventDeadbandSizeEstimationFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventDeadbandSizeEstimationFailed(uint16_t collected, uint16_t expected) :
        ErrorEventBuilder(Error::ModuleID::MotorController, SubmoduleID::Calibration, CodeID::DeadbandSizeEstimation, Error::ErrorSeverity::Trace)
    {
        appendPayload(collected);
        appendPayload(expected);
    }
};

/** <API_REF>
 * @error 0x08020301 LatencyTimeout
 * @desc Failed to estimate the feedback latency of a motor, no movement detected before timeout limit reached.
 * @fix Check motor, analog reader, and motor driver wiring and ensure that everything is properly connected.
 * @payload timeout int32 The timeout value in milliseconds that was reached during the latency estimation process.
 */
class ErrorEventLatencyTimeout : public Error::ErrorEventBuilder
{
public:
    ErrorEventLatencyTimeout(int32_t timeout) :
        ErrorEventBuilder(Error::ModuleID::MotorController, SubmoduleID::Calibration, CodeID::LatencyTimeout, Error::ErrorSeverity::Trace)
    {
        appendPayload(timeout);
    }
};

/** <API_REF>
 * @error 0x08020401 PhysicalBoundNotFound
 * @desc Failed to detect a physical bound of a motor, reached duty cycle limit without finding bound.
 * @fix Check motor assembly and ensure that the motor is centered, and check power board wiring.
 * @payload direction int8 The direction in which the bound was being searched (1 for max, -1 for min).
 * @payload bound_value float32 The duty cycle value at which the search was stopped due to reaching the limit.
 */
class ErrorEventPhysicalBoundNotFound : public Error::ErrorEventBuilder
{
public:
    ErrorEventPhysicalBoundNotFound(int8_t direction, float bound_value) :
        ErrorEventBuilder(Error::ModuleID::MotorController, SubmoduleID::Calibration, CodeID::PhysicalBoundNotFound, Error::ErrorSeverity::Trace)
    {
        appendPayload(direction);
        appendPayload(bound_value);
    }
};

/** <API_REF>
 * @error 0x08020503 CalibrationFailed
 * @desc Motor calibration sequence failed. See above errors for more details.
 * @fix Check motor assembly, wiring, and ensure that the motor is centered and properly connected.
 */
class ErrorEventMotorCalibrationFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventMotorCalibrationFailed(uint8_t motor_channel, uint8_t analog_channel) :
        ErrorEventBuilder(Error::ModuleID::MotorController, SubmoduleID::Calibration, CodeID::CalibrationFailed, Error::ErrorSeverity::Error)
    {
        appendPayload(motor_channel);
        appendPayload(analog_channel);
    }
};