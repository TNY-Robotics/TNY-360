#pragma once
#include "common/Error.hpp"
#include "esp_err.h"

enum class SubmoduleID : uint8_t
{
    Initialization = 0x01,
    Runtime = 0x02,
    Cleanup = 0x03,
};

enum class CodeID : uint8_t
{
    // Initialization
    GPIOConfigFailed = 0x01,
    OneshotInitFailed = 0x02,
    OneshotConfigFailed = 0x03,
    CalibrationInitFailed = 0x04,

    // Runtime
    ADCReadFailed = 0x01,
    ConversionFailed = 0x02,
    GPIOSelectFailed = 0x03,

    // Cleanup
    OneshotDeleteFailed = 0x01,
    CalibrationDeleteFailed = 0x02,
};

/** <API_REF>
 * @error 0x11010101 GPIOConfigFailed
 * @desc Indicates that an error occurred while configuring the GPIO pins for the ADC multiplexer.
 * @fix Check for any GPIO configuration issues and ensure the GPIO pins are properly initialized.
 * @payload err uint32 The esp_err_t error code returned by the GPIO configuration operation.
 */
class ErrorEventGPIOConfigFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventGPIOConfigFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Initialization, CodeID::GPIOConfigFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11010201 OneshotInitFailed
 * @desc Indicates that an error occurred while initializing the ADC oneshot unit.
 * @fix Check for any ADC configuration issues and ensure the ADC is properly initialized.
 * @payload err uint32 The esp_err_t error code returned by the ADC oneshot initialization operation.
 */
class ErrorEventOneshotInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventOneshotInitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Initialization, CodeID::OneshotInitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11010301 OneshotConfigFailed
 * @desc Indicates that an error occurred while configuring the ADC oneshot channel.
 * @fix Check for any ADC configuration issues and ensure the ADC channel is properly initialized.
 * @payload err uint32 The esp_err_t error code returned by the ADC oneshot channel configuration operation.
 */
class ErrorEventOneshotConfigFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventOneshotConfigFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Initialization, CodeID::OneshotConfigFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11010401 CalibrationInitFailed
 * @desc Indicates that an error occurred while creating the ADC calibration handle for curve fitting.
 * @fix Check for any ADC calibration issues and ensure the calibration handle is properly initialized.
 * @payload err uint32 The esp_err_t error code returned by the ADC calibration initialization operation.
 */
class ErrorEventCalibrationInitFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCalibrationInitFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Initialization, CodeID::CalibrationInitFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11020101 ADCReadFailed
 * @desc Indicates that an error occurred while reading from the ADC (Analog-to-Digital Converter).
 * @fix Check for any ADC configuration issues and ensure the ADC is properly initialized.
 * @payload err uint32 The esp_err_t error code returned by the ADC read operation.
 */
class ErrorEventADCReadFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventADCReadFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Runtime, CodeID::ADCReadFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11020201 ConversionFailed
 * @desc Indicates that an error occurred while converting the raw ADC value to a voltage.
 * @fix Check for any calibration issues and ensure the ADC calibration is properly initialized.
 * @payload err uint32 The esp_err_t error code returned by the ADC conversion operation.
 */
class ErrorEventConversionFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventConversionFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Runtime, CodeID::ConversionFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11020301 GPIOSelectFailed
 * @desc Indicates that an error occurred while selecting the ADC channel using GPIO pins.
 * @fix Check for any GPIO configuration issues and ensure the GPIO pins are properly initialized.
 * @payload err uint32 The esp_err_t error code returned by the GPIO selection operation.
 */
class ErrorEventGPIOSelectFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventGPIOSelectFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Runtime, CodeID::GPIOSelectFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11030101 OneshotDeleteFailed
 * @desc Indicates that an error occurred while deleting the ADC oneshot unit.
 * @fix Check for any ADC configuration issues and ensure the ADC is properly deinitialized.
 * @payload err uint32 The esp_err_t error code returned by the ADC oneshot deletion operation.
 */
class ErrorEventOneshotDeleteFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventOneshotDeleteFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Cleanup, CodeID::OneshotDeleteFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};

/** <API_REF>
 * @error 0x11030201 CalibrationDeleteFailed
 * @desc Indicates that an error occurred while deleting the ADC calibration handle for curve fitting.
 * @fix Check for any ADC calibration issues and ensure the calibration handle is properly deinitialized.
 * @payload err uint32 The esp_err_t error code returned by the ADC calibration deletion operation.
 */
class ErrorEventCalibrationDeleteFailed : public Error::ErrorEventBuilder
{
public:
    ErrorEventCalibrationDeleteFailed(esp_err_t err) :
        ErrorEventBuilder(Error::ModuleID::ADC, SubmoduleID::Cleanup, CodeID::CalibrationDeleteFailed, Error::ErrorLevel::Trace)
    {
        appendPayload(err);
    }
};
