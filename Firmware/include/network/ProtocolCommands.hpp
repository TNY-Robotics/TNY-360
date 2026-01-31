/**
 * HEY ! This file is gonna get pretty big, and it could be a good idea to split it later.
 **/

#pragma once
#include "network/Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/Log.hpp"
#include "Robot.hpp"

using CallbackResolver = void(*)(const Protocol::Response& res);
using HandlerCallback = void(*)(const Protocol::Request& req, CallbackResolver resolve);

typedef struct CommandHandler
{
    uint8_t cmd;
    uint8_t len;
    HandlerCallback callback;
} CommandHandler;

CommandHandler handlers[] = {
    // ping command (status: bool true if all good)
    { 0x00, 0, [](const Protocol::Request& req, CallbackResolver resolve) {
        // Maybe we should acutally check something (Like Robot::Status in the future).
        resolve(Protocol::Response(req.id, true));
    }},
    // start whole body calibration
    { 0x01, 0, [](const Protocol::Request& req, CallbackResolver resolve) {
        Error err = Robot::GetInstance().getBody().startCalibration();
        resolve(Protocol::Response(req.id, err == Error::None));
    }},
    // start joint calibration
    { 0x02, sizeof(uint8_t), [](const Protocol::Request& req, CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        uint8_t motor_channel;
        if (Error err = reader.read<uint8_t>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Joint* joint = Joint::GetJoint(motor_channel);
        if (joint == nullptr)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Error err = joint->getMotorController().startCalibration();
        resolve(Protocol::Response(req.id, err == Error::None));
    }},

    // get joint state
    { 0x20, sizeof(MotorDriver::Channel), [](const Protocol::Request& req, CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        MotorDriver::Channel motor_channel;
        if (Error err = reader.read<MotorDriver::Channel>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Joint* joint = Joint::GetJoint(motor_channel);
        if (joint == nullptr)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        bool state = joint->isEnabled();
        uint8_t payload[sizeof(bool)];
        memcpy(payload, &state, sizeof(bool));
        resolve(Protocol::Response(req.id, true, payload, sizeof(bool)));
    }},

    // get joint target angle
    { 0x21, sizeof(MotorDriver::Channel), [](const Protocol::Request& req, CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        MotorDriver::Channel motor_channel;
        if (Error err = reader.read<MotorDriver::Channel>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Joint* joint = Joint::GetJoint(motor_channel);
        if (joint == nullptr)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        float target_angle_rad;
        if (Error err = joint->getTarget(target_angle_rad); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        uint8_t payload[sizeof(float)];
        memcpy(payload, &target_angle_rad, sizeof(float));
        resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
    }},

    // get joint current angle
    { 0x22, sizeof(MotorDriver::Channel), [](const Protocol::Request& req, CallbackResolver resolve) {
        BinaryReader reader(req.payload, req.len);
        
        MotorDriver::Channel motor_channel;
        if (Error err = reader.read<MotorDriver::Channel>(motor_channel); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Joint* joint = Joint::GetJoint(motor_channel);
        if (joint == nullptr)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        float current_angle_rad;
        // if (Error err = joint->getPosition(current_angle_rad); err != Error::None)
        if (Error err = joint->getFeedback(current_angle_rad); err != Error::None) // FIXME : Kalman estimation is broken
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        uint8_t payload[sizeof(float)];
        memcpy(payload, &current_angle_rad, sizeof(float));
        resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
    }},

    // get joint feedback angle
    { 0x23, sizeof(uint8_t), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        float feedback_angle_rad;
        if (Error err = joint->getFeedback(feedback_angle_rad); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        uint8_t payload[sizeof(float)];
        memcpy(payload, &feedback_angle_rad, sizeof(float));
        resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
    }},

    // get joint predicted angle
    { 0x24, sizeof(uint8_t), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        float predicted_angle_rad;
        if (Error err = joint->getPrediction(predicted_angle_rad); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }
        uint8_t payload[sizeof(float)];
        memcpy(payload, &predicted_angle_rad, sizeof(float));
        resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
    }},

    // get joint calibration state
    { 0x25, sizeof(uint8_t), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        MotorController::CalibrationState calib_state = joint->getMotorController().getCalibrationState();
        uint8_t result = static_cast<uint8_t>(calib_state);
        resolve(Protocol::Response(req.id, true, &result, sizeof(uint8_t)));
    }},

    // get joint calibration progress
    { 0x26, sizeof(uint8_t), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        float result = joint->getMotorController().getCalibrationProgress();
        uint8_t payload[sizeof(float)];
        memcpy(payload, &result, sizeof(float));
        resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
    }},

    // get all joint angles
    { 0x27, 0, [](const Protocol::Request& req, CallbackResolver resolve) {
        Joint* joints[] = {
            &Robot::GetInstance().getBody().getFrontLeftLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getFrontLeftLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getFrontLeftLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getFrontRightLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getBackLeftLeg().getKneePitch(),
            &Robot::GetInstance().getBody().getBackRightLeg().getHipRoll(),
            &Robot::GetInstance().getBody().getBackRightLeg().getHipPitch(),
            &Robot::GetInstance().getBody().getBackRightLeg().getKneePitch()
        };
        float angles_rad[12];
        for (size_t i = 0; i < 12; i++)
        {
            if (Error err = joints[i]->getPosition(angles_rad[i]); err != Error::None)
            {
                resolve(Protocol::Response(req.id, false));
                return;
            }
        }
        uint8_t payload[sizeof(float)*12];
        memcpy(payload, angles_rad, sizeof(float)*12);
        resolve(Protocol::Response(req.id, true, payload, sizeof(float)*12));
    }},

    // set joint state (enabled/disabled)
    { 0x60, sizeof(uint8_t) + sizeof(bool), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        Error err = enable ? joint->enable() : joint->disable();
        resolve(Protocol::Response(req.id, (err == Error::None)));
    }},

    // set joint target angle
    { 0x61, sizeof(uint8_t) + sizeof(float), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        Error err = joint->setTarget(target_angle_rad);
        resolve(Protocol::Response(req.id, (err == Error::None)));
    }},
    
    // set joint target angle with timing
    { 0x62, sizeof(uint8_t) + sizeof(float) + sizeof(float), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        Error err = joint->setTarget_Timed(target_angle_rad, time_s);
        resolve(Protocol::Response(req.id, (err == Error::None)));
    }},
    
    // set leg target position
    { 0x63, sizeof(uint8_t) + sizeof(float)*3, [](const Protocol::Request& req, CallbackResolver resolve) {
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

        Error err = legs[leg_index]->setTarget(target_pos);
        resolve(Protocol::Response(req.id, (err == Error::None)));
    }},
    
    // set leg target position timed
    { 0x64, sizeof(uint8_t) + sizeof(float)*3 + sizeof(float), [](const Protocol::Request& req, CallbackResolver resolve) {
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

        Error err = legs[leg_index]->setTarget_Timed(target_pos, time_s);
        resolve(Protocol::Response(req.id, (err == Error::None)));
    }},
    
    // set body posture
    { 0x65, sizeof(float)*3 + sizeof(float)*3, [](const Protocol::Request& req, CallbackResolver resolve) {
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
        Error err = Robot::GetInstance().getBody().setPosture(target_posture);
        resolve(Protocol::Response(req.id, (err == Error::None)));
    }},
    
    // set feet position
    { 0x66, sizeof(uint8_t) + sizeof(float)*3, [](const Protocol::Request& req, CallbackResolver resolve) {
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

        Error err = Robot::GetInstance().getBody().setFeetPosition(static_cast<Body::LegIndex>(leg_index), position);
        resolve(Protocol::Response(req.id, (err == Error::None)));
    }}
};