#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/RPC.hpp"
#include "locomotion/Leg.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Leg
{
    constexpr uint8_t MODULE_ID = 0x04;

    /** <API_REF>
     * @module leg 0x04
     * @action setEnabled 0x00
     * @desc Sets the enabled state of the joints in a leg.
     * @arg leg_id uint8 ID of the leg to set the enabled state for.
     * @arg enabled_flag uint8 Bitfield representing the enabled state of each joint (bit 0 = hip, bit 1 = knee, bit 2 = ankle).
     * @impl done
     */
    static void SetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Status::Ok || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        uint8_t enabledFlag;
        if (reader.read(enabledFlag) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([enabledFlag, legId](){
            Status err;
            for (int i = 0; i < (int) ::Leg::JointId::Count; i++)
            {
                bool isEnabled = (enabledFlag & (1 << i)) != 0;
                Joint& joint = Robot::GetInstance().getBody().getLeg((::Leg::Id)legId).getJoint((::Leg::JointId)i);
                if (isEnabled) err = joint.enable();
                else err = joint.disable();
                if (err != Status::Ok) return err;
            }
            return Status::Ok;
        }, [ctx](Status err){
            if (err != Status::Ok)
                ctx.respond(ResponseStatus::InvalidParameters);
            else ctx.respond(ResponseStatus::Ok);
        });
    }

    /** <API_REF>
     * @module leg 0x04
     * @action getEnabled 0x01
     * @desc Gets the enabled state of the joints in a leg.
     * @arg leg_id uint8 ID of the leg to get the enabled state for.
     * @result enabled_flag uint8 Bitfield representing the enabled state of each joint (bit 0 = hip, bit 1 = knee, bit 2 = ankle).
     * @impl done
     */
    static void GetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Status::Ok || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<uint8_t>([legId]() {
            uint8_t enabledFlag = 0;
            for (int i = 0; i < (int) ::Leg::JointId::Count; i++)
            {
                Joint& joint = Robot::GetInstance().getBody().getLeg((::Leg::Id)legId).getJoint((::Leg::JointId)i);
                if (joint.isEnabled())
                {
                    enabledFlag |= (1 << i);
                }
            }
            return enabledFlag;
        }, [ctx](uint8_t enabledFlag){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabledFlag, sizeof(enabledFlag));
        });
    }

    /** <API_REF>
     * @module leg 0x04
     * @action setPosition 0x02
     * @desc Sets the target position of a leg.
     * @arg leg_id uint8 ID of the leg to set the position for.
     * @arg x_m float32 X coordinate of the target position in meters.
     * @arg y_m float32 Y coordinate of the target position in meters.
     * @arg z_m float32 Z coordinate of the target position in meters.
     * @arg clear_overrides bool Whether to clear joint overrides when setting the position (default: true).
     * @impl done
     */
    static void SetPosition(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Status::Ok || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float x_m, y_m, z_m;
        if (reader.read(x_m) != Status::Ok || reader.read(y_m) != Status::Ok || reader.read(z_m) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        bool clearOverrides;
        if (reader.read(clearOverrides) != Status::Ok)
        {
            clearOverrides = true; // clear overrides by default
        }

        if (clearOverrides)
        {
            for (int i = 0; i < (int) ::Leg::JointId::Count; i++)
            {
                Joint::Id joint_id = static_cast<Joint::Id>(legId * (int) ::Leg::JointId::Count + i);
                Robot::GetInstance().getDecisionLoop().askJointAngle(joint_id, 0, IPC::OverrideMode::None);
            }
        }

        Robot::GetInstance().getDecisionLoop().askLegPosition((::Leg::Id) legId, x_m, y_m, z_m);
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module leg 0x04
     * @action getTargetPosition 0x03
     * @desc Gets the target position of a leg.
     * @arg leg_id uint8 ID of the leg to get the target position for.
     * @result pos Vec3f Target position of the leg in meters.
     * @impl done
     */
    static void GetTargetPosition(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t legId;
        if (reader.read(legId) != Status::Ok || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        const Vec3f& pos = Robot::GetInstance().getDecisionLoop().getControlIntent().leg_overrides[(int) legId].value_pos;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &pos, sizeof(pos));
    }

    /** <API_REF>
     * @module leg 0x04
     * @action getGrounded 0x04
     * @desc Gets whether a leg is currently grounded.
     * @arg leg_id uint8 ID of the leg to get the grounded state for.
     * @result grounded bool Whether the leg is currently grounded.
     * @impl done
     */
    static void GetGrounded(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        uint8_t legId;
        if (reader.read(legId) != Status::Ok || legId >= (int) ::Leg::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        bool grounded = Robot::GetInstance().getBody().getLeg((::Leg::Id) legId).isGrounded();
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &grounded, sizeof(grounded));
    }


    static ActionCallback actions[] = {
        SetEnabled,                // 0x00
        GetEnabled,                // 0x01
        SetPosition,               // 0x02
        GetTargetPosition,         // 0x03
        GetGrounded,               // 0x04
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}