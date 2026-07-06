#pragma once
#include "common/utils.hpp"

namespace Diagnostic
{
    Status Init();
    
    Status Denit();

    void SetDiagnosticMode(bool enable);

    bool IsDiagnosticModeEnabled();

    namespace Check
    {
        uint8_t LED();
        uint8_t I2C();
        uint8_t Speaker();
        uint8_t Microphone();
        uint8_t Camera();
        uint8_t Laser();
        uint8_t IMU();
        uint8_t Power();
        uint8_t MotorDriver();
        uint8_t AnalogReader();
        uint8_t ControlLoop();
        uint8_t DecisionLoop();
    }
}