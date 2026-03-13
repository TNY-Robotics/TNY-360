#pragma once
#include "../Protocol.hpp"
#include "common/BinaryReader.hpp"
#include "common/RPC.hpp"
#include "common/SysStats.hpp"
#include "Robot.hpp"

Protocol::CommandHandler systems[] = {
    // ping command (status: bool true if all good)
    { 0x00, 0, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        // Maybe we should acutally check something (Like Robot::Status in the future).
        resolve(Protocol::Response(req.id, true));
    }},
    // start whole body calibration
    { 0x01, 0, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        // Error err = Robot::GetInstance().getBody().startCalibration();
        resolve(Protocol::Response(req.id, false)); // FIXME : Not available anymore
    }},
    // start joint calibration
    { 0x02, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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
    // stop joint calibration
    { 0x03, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        Error err = joint->getMotorController().stopCalibration();
        resolve(Protocol::Response(req.id, err == Error::None));
    }},
    // set joint calibration data
    { 0x04, sizeof(uint8_t) + sizeof(MotorController::CalibrationData), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        MotorController::CalibrationData calib_data;
        if (Error err = reader.read<MotorController::CalibrationData>(calib_data); err != Error::None)
        {
            resolve(Protocol::Response(req.id, false));
            return;
        }

        Error err = joint->getMotorController().setCalibrationData(calib_data);
        resolve(Protocol::Response(req.id, err == Error::None));
    }},
    // get joint calibration data
    { 0x05, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        MotorController::CalibrationData calib_data = joint->getMotorController().getCalibrationData();

        uint8_t payload[sizeof(MotorController::CalibrationData)];
        memcpy(payload, &calib_data, sizeof(MotorController::CalibrationData));
        resolve(Protocol::Response(req.id, true, payload, sizeof(MotorController::CalibrationData)));
    }},
    // delete joint calibration
    { 0x06, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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

        Error err = joint->getMotorController().deleteCalibrationData();
        resolve(Protocol::Response(req.id, err == Error::None));
    }},
    // get joint calibration state
    { 0x07, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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
    { 0x08, sizeof(uint8_t), [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
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
    // Get CPU Usage
    { 0x09, 0, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        SysStats::CPUUsage cpu_usage = SysStats::GetCPUUsage();
        uint8_t payload[sizeof(SysStats::CPUUsage)];
        memcpy(payload, &cpu_usage, sizeof(SysStats::CPUUsage));
        resolve(Protocol::Response(req.id, true, payload, sizeof(SysStats::CPUUsage)));
    }},
    // Get RAM Usage
    { 0x0A, 0, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        SysStats::RAMUsage ram_usage = SysStats::GetRAMUsage();
        uint8_t payload[sizeof(SysStats::RAMUsage)];
        memcpy(payload, &ram_usage, sizeof(SysStats::RAMUsage));
        resolve(Protocol::Response(req.id, true, payload, sizeof(SysStats::RAMUsage)));
    }},
    // Get Temperature
    { 0x0B, 0, [](const Protocol::Request& req, Protocol::CallbackResolver resolve) {
        float temperature = SysStats::GetTemperature();
        uint8_t payload[sizeof(float)];
        memcpy(payload, &temperature, sizeof(float));
        resolve(Protocol::Response(req.id, true, payload, sizeof(float)));
    }},
};