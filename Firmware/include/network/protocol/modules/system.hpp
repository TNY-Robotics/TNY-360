#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/BinaryWriter.hpp"
#include "common/SysStats.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace System
{
    constexpr uint8_t MODULE_ID = 0x00;

    /** <API_REF>
     * @module system 0x00
     * @action ping 0x00
     * @desc Pings the robot to check connection status.
     * @impl done
     */
    static void Ping(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module system 0x00
     * @action reboot 0x01
     * @desc Reboots the robot.
     * @impl done
     */
    static void Reboot(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        esp_restart();
    }

    /** <API_REF>
     * @module system 0x00
     * @action setSettings 0x02
     * @desc Sets the robot's configuration.
     * @arg settings string New settings to apply.
     * @impl nope
     */
    static void SetSettings(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
     * @module system 0x00
     * @action getSettings 0x01
     * @desc Gets the robot's configuration.
     * @result settings string Current settings.
     * @impl nope
     */
    static void GetSettings(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
     * @module system 0x00
     * @action setAutolifeLevel 0x04
     * @desc Sets the autolife level.
     * @arg level uint8 New autolife level (0-100).
     * @impl done
     */
    static void SetAutolifeLevel(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t level;
        if (reader.read(level) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        Error err = Robot::GetInstance().getDecisionLoop().setAutoLifeLevel(level);

        ctx.respond(err == Error::None ? ResponseStatus::Ok : ResponseStatus::InvalidParameters);
    }

    /** <API_REF>
     * @module system 0x00
     * @action getAutolifeLevel 0x05
     * @desc Gets the current autolife level.
     * @result level uint8 Current autolife level (0-100).
     * @impl done
     */
    static void GetAutolifeLevel(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t autoLifeLevel = Robot::GetInstance().getDecisionLoop().getAutoLifeLevel();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &autoLifeLevel, sizeof(autoLifeLevel));
    }

    /** <API_REF>
     * @module system 0x00
     * @action getStatistics 0x06
     * @desc Gets system statistics including temperature, CPU usage, and RAM usage.
     * @result temp_c float32 Current temperature in Celsius.
     * @result cpu_usage CPUUsage Current CPU usage statistics.
     * @result ram_usage RAMUsage Current RAM usage statistics.
     * @impl done
     */
    static void GetStatistics(const RequestContext& ctx, const uint8_t* payload)
    {
        float temp_c = SysStats::GetTemperature();
        SysStats::CPUUsage cpu_usage = SysStats::GetCPUUsage();
        SysStats::RAMUsage ram_usage = SysStats::GetRAMUsage();

        uint8_t result[sizeof(temp_c) + sizeof(cpu_usage) + sizeof(ram_usage)];
        size_t offset = 0;
        memcpy(result + offset, &temp_c, sizeof(temp_c));       offset += sizeof(temp_c);
        memcpy(result + offset, &cpu_usage, sizeof(cpu_usage)); offset += sizeof(cpu_usage);
        memcpy(result + offset, &ram_usage, sizeof(ram_usage)); offset += sizeof(ram_usage);

        ctx.respond(ResponseStatus::Ok, result, sizeof(result));
    }

    /** <API_REF>
     * @module system 0x00
     * @action getNbLogLines 0x07
     * @desc Gets the number of log lines.
     * @result count uint8 Number of log lines.
     * @impl done
     */
    static void GetNbLogLines(const RequestContext& ctx, const uint8_t* payload)
    {
        uint8_t logCount = ::Log::Count();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &logCount, sizeof(logCount));
    }

    /** <API_REF>
     * @module system 0x00
     * @action getLogLine 0x08
     * @desc Gets a specific log line by index.
     * @arg index uint8 Index of the log line to retrieve (0-based).
     * @result timestampMs uint32 Timestamp of the log line in milliseconds since boot.
     * @result level uint8 Log level (0=Debug, 1=Info, 2=Warning, 3=Error).
     * @result indent uint8 Indentation level of the log line.
     * @result tag string Tag associated with the log line.
     * @result message string Log message.
     * @impl done
     */
    static void GetLogLine(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t index;
        if (reader.read(index) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        const ::Log::LineInfo& line = ::Log::GetLine(index);

        uint8_t buffer[256];
        BinaryWriter writer(buffer, sizeof(buffer));
        if (writer.write<uint32_t>(line.timestampMs) != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        if (writer.write<uint8_t>((uint8_t)line.level) != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        if (writer.write<uint8_t>(line.indent) != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        if (writer.writeString(line.tag) != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }
        if (writer.writeString(line.message) != Error::None)
        {
            ctx.respond(ResponseStatus::UnknownError);
            return;
        }

        ctx.respond(ResponseStatus::Ok, buffer, writer.getOffset());
    }

    /** <API_REF>
     * @module system 0x00
     * @action setControlLoopEnabled 0x09
     * @desc Sets the enabled status of the control loop.
     * @arg enabled bool Whether the control loop should be enabled.
     * @impl done
     */
    static void SetControlLoopEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        bool enabled;
        if (reader.read(enabled) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        if (enabled && !Robot::GetInstance().getControlLoop().isRunning())
        {
            if (Robot::GetInstance().getControlLoop().start() != Error::None)
                ctx.respond(ResponseStatus::UnknownError);
        }
        else if (Robot::GetInstance().getControlLoop().isRunning())
        {
            if (Robot::GetInstance().getControlLoop().stop() != Error::None)
                ctx.respond(ResponseStatus::UnknownError);
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module system 0x00
     * @action getControlLoopEnabled 0x0A
     * @desc Gets whether the control loop is currently enabled.
     * @result enabled bool Whether the control loop is currently enabled.
     * @impl done
     */
    static void GetControlLoopEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        bool enabled = Robot::GetInstance().getControlLoop().isRunning();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabled, sizeof(enabled));
    }

    /** <API_REF>
     * @module system 0x00
     * @action setDecisionLoopEnabled 0x0B
     * @desc Sets the enabled status of the decision loop.
     * @arg enabled bool Whether the decision loop should be enabled.
     * @impl done
     */
    static void SetDecisionLoopEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        bool enabled;
        if (reader.read(enabled) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        if (enabled && !Robot::GetInstance().getDecisionLoop().isRunning())
        {
            if (Robot::GetInstance().getDecisionLoop().start() != Error::None)
                ctx.respond(ResponseStatus::UnknownError);
        }
        else if (Robot::GetInstance().getDecisionLoop().isRunning())
        {
            if (Robot::GetInstance().getDecisionLoop().stop() != Error::None)
                ctx.respond(ResponseStatus::UnknownError);
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module system 0x00
     * @action getDecisionLoopEnabled 0x0C
     * @desc Gets whether the decision loop is currently enabled.
     * @result enabled bool Whether the decision loop is currently enabled.
     * @impl done
     */
    static void GetDecisionLoopEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        bool enabled = Robot::GetInstance().getDecisionLoop().isRunning();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabled, sizeof(enabled));
    }


    static ActionCallback actions[] = {
        Ping,                      // 0x00
        Reboot,                    // 0x01
        SetSettings,               // 0x02
        GetSettings,               // 0x03
        SetAutolifeLevel,          // 0x04
        GetAutolifeLevel,          // 0x05
        GetStatistics,             // 0x06
        GetNbLogLines,             // 0x07
        GetLogLine,                // 0x08
        SetControlLoopEnabled,     // 0x09
        GetControlLoopEnabled,     // 0x0A
        SetDecisionLoopEnabled,    // 0x0B
        GetDecisionLoopEnabled,    // 0x0C
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}