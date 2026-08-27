#include "decision/DecisionLoop.hpp"
#include "drivers/PowerDriver.hpp"
#include "common/Log.hpp"

// This file contains the two main functions of the Decision Loop : update() and fillIntent().
// One is for updating the internal logic of the decision loop
// The other is for filling the intent object that will be sent to the Reflex core based on the decision state

void DecisionLoop::update(float dt, const IPC::RobotState& state)
{
    // TODO : Implement
    // PowerDriver::ReadData();
    // PowerDriver::Data data = PowerDriver::GetData();
    // LOG_INFO(TAG, "Voltage: {%.2f} V, Current: {%.2f} A, Power: {%.2f} W", data.voltage_v, data.current_a, data.power_w);
    // NOTE for future battery estimation :
    // - The BMS on the battery pack cuts power at 9V

    // AnalogDriver::Value voltage;
    // AnalogDriver::GetVoltage(0, voltage);
    // printf(">Voltage:%.4f\n", voltage);
}

void DecisionLoop::fillIntent(IPC::ControlIntent& intent, const IPC::RobotState& state)
{
    // TODO : Implement
}