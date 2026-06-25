#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/Log.hpp"
#include "common/RPC.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Body
{
    constexpr uint8_t MODULE_ID = 0x03;

    /** <API_REF>
     * @module body 0x03
     * @action setEnabled 0x00
     * @desc Sets the enabled state of the body joints.
     * @arg enabled_flag uint16 Bitfield representing the enabled state of each joint (see JointId).
     * @impl done
     */
    static void SetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        uint16_t enabledFlag;
        if (reader.read(enabledFlag) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Status>([enabledFlag](){
            Status err;
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                bool isEnabled = (enabledFlag & (1 << i)) != 0;
                Joint* joint = Joint::GetJoint((Joint::Id) i);
                if (joint == nullptr) continue;
                if (isEnabled) err = joint->enable();
                else err = joint->disable();
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
     * @module body 0x03
     * @action getEnabled 0x01
     * @desc Gets the enabled state of the body joints.
     * @result enabled_flag uint16 Bitfield representing the enabled state of each joint (see JointId).
     * @impl done
     */
    static void GetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<uint16_t>([]() {
            uint16_t enabledFlag = 0;
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                Joint* joint = Joint::GetJoint((Joint::Id) i);
                if (joint != nullptr && joint->isEnabled())
                {
                    enabledFlag |= (1 << i);
                }
            }
            return enabledFlag;
        }, [ctx](uint16_t enabledFlag){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabledFlag, sizeof(enabledFlag));
        });
    }

    /** <API_REF>
     * @module body 0x03
     * @action setVelocity 0x02
     * @desc Sets the target velocity of the robot's body.
     * @arg x_ms float32 X velocity in meters per second.
     * @arg y_ms float32 Y velocity in meters per second.
     * @arg z_rads float32 Rotational velocity around Z axis in radians per second.
     * @arg clear_overrides bool Whether to clear joint overrides when setting the velocity (default: true).
     * @impl done
     */
    static void SetVelocity(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);
        
        float x_ms, y_ms, z_rads;
        if (reader.read(x_ms) != Status::Ok || reader.read(y_ms) != Status::Ok || reader.read(z_rads) != Status::Ok)
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
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                Robot::GetInstance().getDecisionLoop().askJointAngle((Joint::Id) i, 0, IPC::OverrideMode::None);
            }
        }
        Robot::GetInstance().getDecisionLoop().askBodyVelocity(x_ms, y_ms, z_rads);
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module body 0x03
     * @action getTargetVelocity 0x03
     * @desc Gets the target velocity of the robot's body.
     * @result x_ms float32 X velocity in meters per second.
     * @result y_ms float32 Y velocity in meters per second.
     * @result z_rads float32 Rotational velocity around Z axis in radians per second.
     * @impl done
     */
    static void GetTargetVelocity(const RequestContext& ctx, const uint8_t* payload)
    {
        Vec3f vel = Robot::GetInstance().getDecisionLoop().getControlIntent().body_vel;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &vel, sizeof(vel));
    }

    /** <API_REF>
     * @module body 0x03
     * @action setPosture 0x04
     * @desc Sets the target posture of the robot's body.
     * @arg x_pos float32 X position in meters.
     * @arg y_pos float32 Y position in meters.
     * @arg z_pos float32 Z position in meters.
     * @arg x_rot float32 X rotation in radians (Euler XYZ).
     * @arg y_rot float32 Y rotation in radians (Euler XYZ).
     * @arg z_rot float32 Z rotation in radians (Euler XYZ).
     * @arg clear_overrides bool Whether to clear joint overrides when setting the posture (default: true).
     * @impl done
     */
    static void SetPosture(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        float x_pos, y_pos, z_pos;
        if (reader.read(x_pos) != Status::Ok || reader.read(y_pos) != Status::Ok || reader.read(z_pos) != Status::Ok)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        float x_rot, y_rot, z_rot;
        if (reader.read(x_rot) != Status::Ok || reader.read(y_rot) != Status::Ok || reader.read(z_rot) != Status::Ok)
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
            for (int i = 0; i < (int) Joint::Id::Count; i++)
            {
                Robot::GetInstance().getDecisionLoop().askJointAngle((Joint::Id) i, 0, IPC::OverrideMode::None);
            }
        }
        Robot::GetInstance().getDecisionLoop().askBodyPosition(x_pos, y_pos, z_pos);
        Robot::GetInstance().getDecisionLoop().askBodyRotation(x_rot, y_rot, z_rot);
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module body 0x03
     * @action getPosture 0x05
     * @desc Gets the target posture of the robot's body.
     * @result pos Vec3f Target position of the body in meters.
     * @result rot Vec3f Target rotation of the body in radians (Euler XYZ).
     * @impl done
     */
    static void GetPosture(const RequestContext& ctx, const uint8_t* payload)
    {
        const IPC::ControlIntent& intent = Robot::GetInstance().getDecisionLoop().getControlIntent();

        struct Posture {
            Vec3f pos;
            Vec3f rot;
        } posture;
        posture.pos = intent.body_pos;
        posture.rot = intent.body_rot;

        ctx.respond(ResponseStatus::Ok, (uint8_t*) &posture, sizeof(posture));
    }


    static ActionCallback actions[] = {
        SetEnabled,                // 0x00
        GetEnabled,                // 0x01
        SetVelocity,               // 0x02
        GetTargetVelocity,         // 0x03
        SetPosture,                // 0x04
        GetPosture,                // 0x05
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}