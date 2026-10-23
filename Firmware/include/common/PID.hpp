#pragma once
#include "common/utils.hpp"

class PID
{
public:
    PID() : kp(0.f), ki(0.f), kd(0.f), integral(0.f), previous_error(0.f) {}
    PID(float kp, float ki, float kd) : kp(kp), ki(ki), kd(kd), integral(0.f), previous_error(0.f) {}

    float compute(float target, float current, float dt)
    {
        float error = target - current;
        integral += error * dt;
        float derivative = (error - previous_error) / dt;
        previous_error = error;

        return (kp * error) + (ki * integral) + (kd * derivative);
    }

    void reset()
    {
        integral = 0.f;
        previous_error = 0.f;
    }

private:
    float kp;
    float ki;
    float kd;

    float integral;
    float previous_error;
};