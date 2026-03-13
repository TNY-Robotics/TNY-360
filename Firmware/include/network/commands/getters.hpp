#pragma once
#include "../Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/RPC.hpp"
#include "drivers/MotorDriver.hpp"

Protocol::CommandHandler getters[] = {
    // get joint state
    { 0x20, sizeof(MotorDriver::Channel), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        struct RPCResult { Error err; bool data; };
        RPC::ExecuteThreadSafe<RPCResult>(
            [joint]() -> RPCResult { // task getter, executing on core 1
                bool state = joint->isEnabled();
                return { Error::None, state };
            }, 
            [req, resolve](RPCResult res) { // resolve callback
                if (res.err != Error::None) {
                    resolve(Protocol::Response(req.id, false));
                } else {
                    uint8_t payload[sizeof(bool)];
                    memcpy(payload, &res.data, sizeof(bool));
                    resolve(Protocol::Response(req.id, true, payload, sizeof(bool)));
                }
            }
        );
    }},
    // get joint target angle
    { 0x21, sizeof(MotorDriver::Channel), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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
        
        struct RPCResult { Error err; float data; };
        RPC::ExecuteThreadSafe<RPCResult>(
            [joint]() -> RPCResult { // task getter, executing on core 1
                float feedback;
                Error err = joint->getTarget(feedback);
                return { err, feedback };
            }, 
            [req, resolve](RPCResult res) { // resolve callback
                if (res.err != Error::None) {
                    resolve(Protocol::Response(req.id, false));
                } else {
                    uint8_t payload[sizeof(float)];
                    memcpy(payload, &res.data, sizeof(float));
                    resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
                }
            }
        );
    }},
    // get joint current angle
    { 0x22, sizeof(MotorDriver::Channel), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        struct RPCResult { Error err; float data; };
        RPC::ExecuteThreadSafe<RPCResult>(
            [joint]() -> RPCResult { // task getter, executing on core 1
                float feedback;
                Error err = joint->getPrediction(feedback);
                return { err, feedback };
            }, 
            [req, resolve](RPCResult res) { // resolve callback

                if (res.err != Error::None) {
                    resolve(Protocol::Response(req.id, false));
                } else {
                    uint8_t payload[sizeof(float)];
                    memcpy(payload, &res.data, sizeof(float));
                    resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
                }
            }
        );
    }},
    // get joint feedback angle
    { 0x23, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        struct RPCResult { Error err; float data; };
        RPC::ExecuteThreadSafe<RPCResult>(
            [joint]() -> RPCResult { // task getter, executing on core 1
                float feedback;
                Error err = joint->getFeedback(feedback);
                return { err, feedback };
            }, 
            [req, resolve](RPCResult res) { // resolve callback
                if (res.err != Error::None) {
                    resolve(Protocol::Response(req.id, false));
                } else {
                    uint8_t payload[sizeof(float)];
                    memcpy(payload, &res.data, sizeof(float));
                    resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
                }
            }
        );
    }},
    // get joint predicted angle
    { 0x24, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        struct RPCResult { Error err; float data; };
        RPC::ExecuteThreadSafe<RPCResult>(
            [joint]() -> RPCResult { // task getter, executing on core 1
                float feedback;
                Error err = joint->getPrediction(feedback);
                return { err, feedback };
            }, 
            [req, resolve](RPCResult res) { // resolve callback
                if (res.err != Error::None) {
                    resolve(Protocol::Response(req.id, false));
                } else {
                    uint8_t payload[sizeof(float)];
                    memcpy(payload, &res.data, sizeof(float));
                    resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
                }
            }
        );
    }},
    // get body orientation
    { 0x25, 0, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        struct RPCResult { Quatf data; };
        RPC::ExecuteThreadSafe<RPCResult>(
            []() -> RPCResult { // task getter, executing on core 1
                Quatf& orientation = Robot::GetInstance().getBody().getIMU().getOrientation();
                return { orientation };
            }, 
            [req, resolve](RPCResult res) { // resolve callback
                uint8_t payload[sizeof(float)*4];
                memcpy(payload, &res.data.x, sizeof(float));
                memcpy(payload + sizeof(float), &res.data.y, sizeof(float));
                memcpy(payload + sizeof(float)*2, &res.data.z, sizeof(float));
                memcpy(payload + sizeof(float)*3, &res.data.w, sizeof(float));
                resolve(Protocol::Response(req.id, true, payload, sizeof(float)*4));
            }
        );
    }},
};