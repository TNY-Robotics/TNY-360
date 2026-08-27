#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/RPC.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Gait
{
    constexpr uint8_t MODULE_ID = 0x02;

    /** <API_REF>
     * @module gait 0x02
     * @action setType 0x00
     * @desc Sets the gait type for the robot.
     * @arg gait_type uint8 Gait type (0 = Stand, 1 = Walk, 2 = Trot, 3 = Gallop).
     * @impl done
     */
    static void SetType(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t gaitType;
        if (reader.read(gaitType) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([gaitType]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            GaitPlanner::GaitConfig config = planner.getConfig();
            config.gait_type = static_cast<GaitPlanner::GaitType>(gaitType);
            planner.setConfig(config);
            return Status::Ok;
        }, [ctx](Status status) {
            ctx.respond(ResponseStatus::Ok);
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action getType 0x01
     * @desc Gets the current gait type for the robot.
     * @result gait_type uint8 Gait type (0 = Stand, 1 = Walk, 2 = Trot, 3 = Gallop).
     * @impl done
     */
    static void GetType(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<GaitPlanner::GaitType>([]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            return planner.getConfig().gait_type;
        }, [ctx](GaitPlanner::GaitType gaitType) {
            uint8_t gaitTypeValue = static_cast<uint8_t>(gaitType);
            ctx.respond(ResponseStatus::Ok, (uint8_t*)&gaitTypeValue, sizeof(gaitTypeValue));
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action setFrequency 0x02
     * @desc Sets the step frequency for the robot's gait.
     * @arg frequency float Step frequency in Hz.
     * @impl done
     */
    static void SetFrequency(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        float frequency;
        if (reader.read(frequency) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([frequency]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            GaitPlanner::GaitConfig config = planner.getConfig();
            config.step_freq_hz = frequency;
            planner.setConfig(config);
            return Status::Ok;
        }, [ctx](Status status) {
            ctx.respond(ResponseStatus::Ok);
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action getFrequency 0x03
     * @desc Gets the step frequency of the robot's gait.
     * @result frequency float Step frequency in Hz.
     * @impl done
     */
    static void GetFrequency(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<float>([]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            return planner.getConfig().step_freq_hz;
        }, [ctx](float frequency) {
            ctx.respond(ResponseStatus::Ok, (uint8_t*)&frequency, sizeof(frequency));
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action setDutyFactor 0x04
     * @desc Sets the duty factor for the robot's gait.
     * @arg value float Duty factor.
     * @impl done
     */
    static void SetDutyFactor(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        float value;
        if (reader.read(value) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([value]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            GaitPlanner::GaitConfig config = planner.getConfig();
            config.duty_factor = value;
            planner.setConfig(config);
            return Status::Ok;
        }, [ctx](Status status) { ctx.respond(ResponseStatus::Ok); });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action getDutyFactor 0x05
     * @desc Gets the duty factor of the robot's gait.
     * @result value float Duty factor.
     * @impl done
     */
    static void GetDutyFactor(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<float>([]() {
            return Robot::GetInstance().getControlLoop().getGaitPlanner().getConfig().duty_factor;
        }, [ctx](float value) {
            ctx.respond(ResponseStatus::Ok, (uint8_t*)&value, sizeof(value));
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action setStepHeight 0x06
     * @desc Sets the step height for the robot's gait.
     * @arg value float Step height in meters.
     * @impl done
     */
    static void SetStepHeight(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        float value;
        if (reader.read(value) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([value]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            GaitPlanner::GaitConfig config = planner.getConfig();
            config.step_height_m = value;
            planner.setConfig(config);
            return Status::Ok;
        }, [ctx](Status status) {
            ctx.respond(ResponseStatus::Ok);
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action getStepHeight 0x07
     * @desc Gets the step height of the robot's gait.
     * @result value float Step height in meters.
     * @impl done
     */
    static void GetStepHeight(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<float>([]() {
            return Robot::GetInstance().getControlLoop().getGaitPlanner().getConfig().step_height_m;
        }, [ctx](float value) {
            ctx.respond(ResponseStatus::Ok, (uint8_t*)&value, sizeof(value));
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action setStanceDepth 0x08
     * @desc Sets the stance depth for the robot's gait.
     * @arg value float Stance depth in meters.
     * @impl done
     */
    static void SetStanceDepth(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        float value;
        if (reader.read(value) != Status::Ok) {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([value]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            GaitPlanner::GaitConfig config = planner.getConfig();
            config.stance_depth_m = value;
            planner.setConfig(config);
            return Status::Ok;
        }, [ctx](Status status) {
            ctx.respond(ResponseStatus::Ok);
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action getStanceDepth 0x09
     * @desc Gets the stance depth of the robot's gait.
     * @result value float Stance depth in meters.
     * @impl done
     */
    static void GetStanceDepth(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<float>([]() {
            return Robot::GetInstance().getControlLoop().getGaitPlanner().getConfig().stance_depth_m;
        }, [ctx](float value) {
            ctx.respond(ResponseStatus::Ok, (uint8_t*)&value, sizeof(value));
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action setLegSpread 0x0A
     * @desc Sets the leg spread for the robot's gait.
     * @arg spread_x float Spread in the X direction (meters).
     * @arg spread_y float Spread in the Y direction (meters).
     * @impl done
     */
    static void SetLegSpread(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        float spread_x, spread_y;
        if (reader.read(spread_x) != Status::Ok || reader.read(spread_y) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([spread_x, spread_y]() {
            GaitPlanner& planner = Robot::GetInstance().getControlLoop().getGaitPlanner();
            GaitPlanner::GaitConfig config = planner.getConfig();
            config.leg_spread_m = Vec2f(spread_x, spread_y);
            planner.setConfig(config);
            return Status::Ok;
        }, [ctx](Status status) {
            ctx.respond(ResponseStatus::Ok);
        });
    }

    /** <API_REF>
     * @module gait 0x02
     * @action getLegSpread 0x0B
     * @desc Gets the leg spread of the robot's gait.
     * @result spread_x float Spread in the X direction (meters).
     * @result spread_y float Spread in the Y direction (meters).
     * @impl done
     */
    static void GetLegSpread(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<Vec2f>([]() {
            return Robot::GetInstance().getControlLoop().getGaitPlanner().getConfig().leg_spread_m;
        }, [ctx](Vec2f value) {
            ctx.respond(ResponseStatus::Ok, (uint8_t*)&value, sizeof(value));
        });
    }

    static ActionCallback actions[] = {
        SetType,                   // 0x00
        GetType,                   // 0x01
        SetFrequency,              // 0x02
        GetFrequency,              // 0x03
        SetDutyFactor,             // 0x04
        GetDutyFactor,             // 0x05
        SetStepHeight,             // 0x06
        GetStepHeight,             // 0x07
        SetStanceDepth,            // 0x08
        GetStanceDepth,            // 0x09
        SetLegSpread,              // 0x0A
        GetLegSpread,              // 0x0B
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}