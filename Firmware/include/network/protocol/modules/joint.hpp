#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "locomotion/Joint.hpp"
#include "Robot.hpp"
#include "common/Log.hpp"
#include "common/RPC.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace Joint
{
    constexpr uint8_t MODULE_ID = 0x05;

    /** <API_REF>
     * @module joint 0x05
     * @action setEnabled 0x00
     * @desc Sets the enabled state of a joint.
     * @arg joint_id uint8 ID of the joint to set the enabled state for.
     * @arg enabled bool Whether to enable (true) or disable (false) the joint.
     * @impl done
     */
    static void SetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        bool enabled;
        if (reader.read(enabled) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<Error>([joint, enabled](){
            Error err;
            if (enabled) err = joint->enable();
            else err = joint->disable();
            return err;
        }, [ctx](Error err){
            if (err != Error::None)
                ctx.respond(ResponseStatus::InvalidParameters);
            else ctx.respond(ResponseStatus::Ok);
        });
    }

    /** <API_REF>
     * @module joint 0x05
     * @action getEnabled 0x01
     * @desc Gets the enabled state of a joint.
     * @arg joint_id uint8 ID of the joint to get the enabled state for.
     * @result enabled bool Whether the joint is currently enabled.
     * @impl done
     */
    static void GetEnabled(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        RPC::ExecuteThreadSafe<bool>([joint](){
            return joint->isEnabled();
        }, [ctx](bool enabled){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &enabled, sizeof(enabled));
        });
    }

    /** <API_REF>
     * @module joint 0x05
     * @action setAngle 0x02
     * @desc Sets the target angle of a joint.
     * @arg joint_id uint8 ID of the joint to set the target angle for.
     * @arg angle float32 Target angle in radians.
     * @impl done
     */
    static void SetAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float angle;
        if (reader.read(angle) != Error::None)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        Robot::GetInstance().getDecisionLoop().askJointAngle((::Joint::Id) jointId, angle);
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module joint 0x05
     * @action getTargetAngle 0x03
     * @desc Gets the target angle of a joint.
     * @arg joint_id uint8 ID of the joint to get the target angle for.
     * @result angle float32 Target angle in radians.
     * @impl done
     */
    static void GetTargetAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }
        
        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float targetAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].target_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &targetAngle, sizeof(targetAngle));
    }

    /** <API_REF>
    * @module joint 0x05
    * @action getFeedbackAngle 0x04
    * @desc Gets the feedback angle of a joint.
    * @arg joint_id uint8 ID of the joint to get the feedback angle for.
    * @result angle float32 Feedback angle in radians.
    * @impl done
    */
    static void GetFeedbackAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float feedbackAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].feedback_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &feedbackAngle, sizeof(feedbackAngle));
    }

    /** <API_REF>
    * @module joint 0x05
    * @action getModelAngle 0x05
    * @desc Gets the model angle of a joint.
    * @arg joint_id uint8 ID of the joint to get the model angle for.
    * @result angle float32 Model angle in radians.
    * @impl done
    */
    static void GetModelAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float modelAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].model_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &modelAngle, sizeof(modelAngle));
    }

    /** <API_REF>
     * @module joint 0x05
     * @action getEstimatedAngle 0x06
     * @desc Gets the estimated angle of a joint.
     * @arg joint_id uint8 ID of the joint to get the estimated angle for.
     * @result angle float32 Estimated angle in radians.
     * @impl done
     */
    static void GetEstimatedAngle(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        uint8_t jointId;
        if (reader.read(jointId) != Error::None || jointId >= (int) ::Joint::Id::Count)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        ::Joint* joint = ::Joint::GetJoint((::Joint::Id) jointId);
        if (joint == nullptr)
        {
            ctx.respond(ResponseStatus::InvalidParameters);
            return;
        }

        float estimatedAngle = Robot::GetInstance().getDecisionLoop().getRobotState().joints[(int) jointId].estimated_angle_rad;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &estimatedAngle, sizeof(estimatedAngle));
    }

    /** <API_REF>
     * @module joint 0x05
     * @action setJointAngles 0x07
     * @desc Sets the target angles of all joints at once.
     * @arg angles float32[14] Target angles for all joints in radians (order: LF hip, LF knee, LF ankle, RF hip, RF knee, RF ankle, LH hip, LH knee, LH ankle, RH hip, RH knee, RH ankle).
     * @impl done
     */
    static void SetJointAngles(const RequestContext& ctx, const uint8_t* payload)
    {
        BinaryReader reader(payload, ctx.expected_len);

        float angles[14];
        for (int i = 0; i < 14; i++)
        {
            if (reader.read(angles[i]) != Error::None)
            {
                ctx.respond(ResponseStatus::InvalidParameters);
                return;
            }
        }

        for (int i = 0; i < 14; i++)
        {
            if (Robot::GetInstance().getDecisionLoop().askJointAngle((::Joint::Id) i, angles[i]) != Error::None)
            {
                ctx.respond(ResponseStatus::InvalidParameters);
                return;
            }
        }
        ctx.respond(ResponseStatus::Ok);
    }

    /** <API_REF>
     * @module joint 0x05
     * @action getJointAngles 0x08
     * @desc Gets the feedback angles of all joints at once.
     * @result angles float32[14] Feedback angles for all joints in radians (order: LF hip, LF knee, LF ankle, RF hip, RF knee, RF ankle, LH hip, LH knee, LH ankle, RH hip, RH knee, RH ankle).
     * @impl done
     */
    static void GetJointAngles(const RequestContext& ctx, const uint8_t* payload)
    {
        float angles[14];
        for (int i = 0; i < 14; i++)
        {
            angles[i] = Robot::GetInstance().getDecisionLoop().getRobotState().joints[i].feedback_angle_rad;
        }
        ctx.respond(ResponseStatus::Ok, (uint8_t*) angles, sizeof(angles));
    }


    static ActionCallback actions[] = {
        SetEnabled,                // 0x00
        GetEnabled,                // 0x01
        SetAngle,                  // 0x02
        GetTargetAngle,            // 0x03
        GetFeedbackAngle,          // 0x04
        GetModelAngle,             // 0x05
        GetEstimatedAngle,         // 0x06
        SetJointAngles,            // 0x07
        GetJointAngles             // 0x08
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}