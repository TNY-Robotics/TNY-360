#pragma once
#include "../Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/RPC.hpp"
#include "Robot.hpp"
#include "decision/DecisionLoop.hpp"

Protocol::CommandHandler setters[] = {
    // set joint state (enabled/disabled)
    { 0x60, sizeof(uint8_t) + sizeof(bool), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        uint8_t motor_channel;
        if (Error err = reader.read<uint8_t>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Joint* joint = Joint::GetJoint(static_cast<MotorDriver::Channel>(motor_channel));
        if (joint == nullptr)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        bool enable;
        if (Error err = reader.read<bool>(enable); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        RPC::ExecuteThreadSafe<Error>(
            [joint, enable]() -> Error { // task getter, executing on core 1
                Error err = enable ? joint->enable() : joint->disable();
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // set joint target angle
    { 0x61, sizeof(uint8_t) + sizeof(float), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        uint8_t motor_channel;
        if (Error err = reader.read<uint8_t>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Joint* joint = Joint::GetJoint(static_cast<MotorDriver::Channel>(motor_channel));
        if (joint == nullptr)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        float target_angle_rad;
        if (Error err = reader.read<float>(target_angle_rad); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        RPC::ExecuteThreadSafe<Error>(
            [joint, target_angle_rad]() -> Error { // task getter, executing on core 1
                Error err = joint->setTarget(target_angle_rad);
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // set joint target angle with timing
    { 0x62, sizeof(uint8_t) + sizeof(float) + sizeof(float), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        uint8_t motor_channel;
        if (Error err = reader.read<uint8_t>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Joint* joint = Joint::GetJoint(static_cast<MotorDriver::Channel>(motor_channel));
        if (joint == nullptr)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        float target_angle_rad;
        if (Error err = reader.read<float>(target_angle_rad); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        float time_s;
        if (Error err = reader.read<float>(time_s); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        RPC::ExecuteThreadSafe<Error>(
            [joint, target_angle_rad, time_s]() -> Error { // task getter, executing on core 1
                Error err = joint->setTarget_Timed(target_angle_rad, time_s);
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // set leg target position
    { 0x63, sizeof(uint8_t) + sizeof(float)*3, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        Leg* legs[4] = { 
            &Robot::GetInstance().getBody().getFrontLeftLeg(),
            &Robot::GetInstance().getBody().getFrontRightLeg(),
            &Robot::GetInstance().getBody().getBackLeftLeg(),
            &Robot::GetInstance().getBody().getBackRightLeg()
        };

        uint8_t leg_index;
        if (Error err = reader.read<uint8_t>(leg_index); err != Error::None || leg_index >= 4)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        Leg* leg = legs[leg_index];

        Vec3f target_pos;
        if (Error err = reader.read<float>(target_pos.x); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(target_pos.y); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(target_pos.z); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        RPC::ExecuteThreadSafe<Error>(
            [leg, target_pos]() -> Error { // task getter, executing on core 1
                Error err = leg->setTarget(target_pos);
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // set leg target position timed
    { 0x64, sizeof(uint8_t) + sizeof(float)*3 + sizeof(float), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        Leg* legs[4] = { 
            &Robot::GetInstance().getBody().getFrontLeftLeg(),
            &Robot::GetInstance().getBody().getFrontRightLeg(),
            &Robot::GetInstance().getBody().getBackLeftLeg(),
            &Robot::GetInstance().getBody().getBackRightLeg()
        };

        uint8_t leg_index;
        if (Error err = reader.read<uint8_t>(leg_index); err != Error::None || leg_index >= 4)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        Leg* leg = legs[leg_index];

        Vec3f target_pos;
        if (Error err = reader.read<float>(target_pos.x); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(target_pos.y); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(target_pos.z); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        float time_s;
        if (Error err = reader.read<float>(time_s); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        RPC::ExecuteThreadSafe<Error>(
            [leg, target_pos, time_s]() -> Error { // task getter, executing on core 1
                Error err = leg->setTarget_Timed(target_pos, time_s);
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // set body posture
    { 0x65, sizeof(float)*3 + sizeof(float)*3, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);

        Vec3f position;
        if (Error err = reader.read<float>(position.x); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(position.y); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(position.z); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Vec3f rotation;
        if (Error err = reader.read<float>(rotation.x); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(rotation.y); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(rotation.z); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Transformf target_posture(position, Quatf::FromEulerAngles(rotation));

        RPC::ExecuteThreadSafe<Error>(
            [target_posture]() -> Error { // task getter, executing on core 1
                Error err = Robot::GetInstance().getBody().setPosture(target_posture);
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // set feet position
    { 0x66, sizeof(uint8_t) + sizeof(float)*3, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);

        uint8_t leg_index;
        if (Error err = reader.read<uint8_t>(leg_index); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Vec3f position;
        if (Error err = reader.read<float>(position.x); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(position.y); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        if (Error err = reader.read<float>(position.z); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        RPC::ExecuteThreadSafe<Error>(
            [leg_index, position]() -> Error { // task getter, executing on core 1
                Error err = Robot::GetInstance().getBody().setFeetPosition(static_cast<Body::LegIndex>(leg_index), position);
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // Set Joint PWM
    { 0x67, sizeof(uint8_t) + sizeof(int), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);

        uint8_t motor_channel;
        if (Error err = reader.read<uint8_t>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        int pwm_value_int;
        if (Error err = reader.read<int>(pwm_value_int); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        MotorDriver::Value pwm_value = static_cast<MotorDriver::Value>(pwm_value_int);

        RPC::ExecuteThreadSafe<Error>(
            [motor_channel, pwm_value]() -> Error { // task getter, executing on core 1
                Error err = MotorDriver::SetPWM(static_cast<MotorDriver::Channel>(motor_channel), pwm_value);
                return err;
            }, 
            [req, resolve](Error err) { // resolve callback
                resolve(Protocol::Response(req.id, (err == Error::None)));
            }
        );
    }},
    // Set movement velocity (x_translation_m_s, y_translation_m_s, rotation_rad_s)
    { 0x68, sizeof(float) * 3, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);

        float x_translation;
        if (Error err = reader.read<float>(x_translation); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        float y_translation;
        if (Error err = reader.read<float>(y_translation); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        float rotation;
        if (Error err = reader.read<float>(rotation); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Robot::GetInstance().getDecisionLoop().setBodyVelocity(Vec3f(x_translation, y_translation, rotation));
        resolve(Protocol::Response(req.id, true));
    }},
};