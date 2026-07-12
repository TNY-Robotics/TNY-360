#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "diagnostic/Diagnostic.hpp"

namespace Protocol
{
namespace Diagnostic
{
    constexpr uint8_t MODULE_ID = 0x14;

    /** <API_REF>
     * @module diagnostic 0x14
     * @action isEnabled 0x00
     * @desc Returns if the robot is currently in diagnostic mode.
     * @result enabled bool Whether diagnostic mode is enabled.
     * @impl done
     */
    static void IsEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        bool enabled = ::Diagnostic::IsDiagnosticModeEnabled();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&enabled, sizeof(enabled));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action RebootInDiagnosticMode 0x01
     * @desc Reboots the robot in diagnostic mode.
     * @impl done
     */
    static void RebootInDiagnosticMode(const RequestContext& ctx, const uint8_t* payload)
    {
        Status status = ::Diagnostic::RebootInDiagnosticMode();
        ctx.respond(status == Status::Ok ? ResponseStatus::Ok : ResponseStatus::UnknownError);
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkLED 0x02
     * @desc Checks the status of the LED module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckLED(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::LED();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkI2C 0x03
     * @desc Checks the status of the I2C module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckI2C(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::I2C();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkSpeaker 0x04
     * @desc Checks the status of the Speaker module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckSpeaker(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::Speaker();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkMicrophone 0x05
     * @desc Checks the status of the Microphone module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckMicrophone(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::Microphone();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkCamera 0x06
     * @desc Checks the status of the Camera module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckCamera(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::Camera();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkLaser 0x07
     * @desc Checks the status of the Laser module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckLaser(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::Laser();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkIMU 0x08
     * @desc Checks the status of the IMU module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckIMU(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::IMU();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkPower 0x09
     * @desc Checks the status of the Power module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckPower(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::Power();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkMotorDriver 0x0A
     * @desc Checks the status of the Motor Driver module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckMotorDriver(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::MotorDriver();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkAnalogReader 0x0B
     * @desc Checks the status of the Analog Reader module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckAnalogReader(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::AnalogReader();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkControlLoop 0x0C
     * @desc Checks the status of the Control Loop module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckControlLoop(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::ControlLoop();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }

    /** <API_REF>
     * @module diagnostic 0x14
     * @action checkDecisionLoop 0x0D
     * @desc Checks the status of the Decision Loop module.
     * @result result uint8_t 0 in case of success, error id otherwise.
     * @impl done
     */
    static void CheckDecisionLoop(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t result = ::Diagnostic::Check::DecisionLoop();
        ctx.respond(ResponseStatus::Ok, (const uint8_t*)&result, sizeof(result));
    }


    static ActionCallback actions[] = {
        IsEnabled, // 0x00
        RebootInDiagnosticMode, // 0x01
        CheckLED, // 0x02
        CheckI2C, // 0x03
        CheckSpeaker, // 0x04
        CheckMicrophone, // 0x05
        CheckCamera, // 0x06
        CheckLaser, // 0x07
        CheckIMU, // 0x08
        CheckPower, // 0x09
        CheckMotorDriver, // 0x0A
        CheckAnalogReader, // 0x0B
        CheckControlLoop, // 0x0C
        CheckDecisionLoop // 0x0D
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}