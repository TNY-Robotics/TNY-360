#pragma once
#include "utils.hpp"

namespace Motor
{
    enum class ForceLevel: uint8_t {
        Off = 0,
        Low = 4,
        Medium = 2,
        High = 1
    };
    enum class Id: uint8_t {
        Shoulder_FL = 0,
        Shoulder_FR = 4,
        Shoulder_BL = 8,
        Shoulder_BR = 12,
        Leg_FL = 1,
        Leg_FR = 5,
        Leg_BL = 9,
        Leg_BR = 13,
        Knee_FL = 2,
        Knee_FR = 6,
        Knee_BL = 10,
        Knee_BR = 14,
        Count = 16
    };

    /**
    * @brief Initializes the Motor module.
    * @return Error code indicating success or failure.
    */
    Error Init();

    /**
    * @brief Sets the target angle for a motor.
    * @param id Motor identifier.
    * @param angle Target angle in degrees.
    * @return Error code.
    */
    Error SetAngle(Id id, float angle);

    /**
    * @brief Sets the target angle for a motor over a specified duration.
    * @param id Motor identifier.
    * @param angle Target angle in degrees.
    * @param duration_s Duration in seconds.
    * @return Error code.
    */
    Error SetAngle(Id id, float angle, float duration_s);

    /**
    * @brief Sets target angles for all motors.
    * @param angles Array of target angles.
    * @return Error code.
    */
    Error SetAngles(const float* angles);

    /**
    * @brief Sets target angles for all motors over specified durations.
    * @param angles Array of target angles.
    * @param durations_s Array of durations in seconds.
    * @return Error code.
    */
    Error SetAngles(const float* angles, const float* durations_s);

    /**
    * @brief Sets target angles for specified motors.
    * @param ids Array of motor identifiers.
    * @param angles Array of target angles.
    * @return Error code.
    */
    Error SetAngles(const Id* ids, const float* angles, uint8_t count);

    /**
    * @brief Sets target angles for specified motors over specified durations.
    * @param ids Array of motor identifiers.
    * @param angles Array of target angles.
    * @param durations_s Array of durations in seconds.
    * @return Error code.
    */
    Error SetAngles(const Id* ids, const float* angles, const float* durations_s, uint8_t count);

    /**
    * @brief Sets the force level for a motor.
    * @param id Motor identifier.
    * @param force Desired force level.
    * @return Error code.
    */
    Error SetForce(Id id, ForceLevel force);

    /**
    * @brief Sets force levels for all motors.
    * @param forces Array of force levels.
    * @return Error code.
    */
    Error SetForces(const ForceLevel* forces);

    /**
    * @brief Sets force levels for specified motors.
    * @param ids Array of motor identifiers.
    * @param forces Array of force levels.
    * @return Error code.
    */
    Error SetForces(const Id* ids, const ForceLevel* forces, uint8_t count);

    /**
    * @brief Gets the force level of a motor.
    * @param id Motor identifier.
    * @param outForce Pointer to store the force level.
    * @return Error code.
    */
    Error GetForce(Id id, ForceLevel* outForce);

    /**
    * @brief Gets force levels of all motors.
    * @param outForces Pointer to array to store force levels.
    * @return Error code.
    */
    Error GetForces(ForceLevel* outForces);

    /**
    * @brief Gets force levels of specified motors.
    * @param ids Array of motor identifiers.
    * @param outForces Pointer to array to store force levels.
    * @return Error code.
    */
    Error GetForces(const Id* ids, ForceLevel* outForces, uint8_t count);

    /**
    * @brief Gets the target angle of a motor.
    * @param id Motor identifier.
    * @param outAngle Pointer to store the target angle.
    * @return Error code.
    */
    Error GetTargetAngle(Id id, float* outAngle);

    /**
    * @brief Gets target angles of all motors.
    * @param outAngles Pointer to array to store target angles.
    * @return Error code.
    */
    Error GetTargetAngles(float* outAngles);

    /**
    * @brief Gets target angles of specified motors.
    * @param ids Array of motor identifiers.
    * @param outAngles Pointer to array to store target angles.
    * @return Error code.
    */
    Error GetTargetAngles(const Id* ids, float* outAngles, uint8_t count);

    /**
    * @brief Gets the current angle of a motor.
    * @param id Motor identifier.
    * @param outAngle Pointer to store the current angle.
    * @return Error code.
    */
    Error GetCurrentAngle(Id id, float* outAngle);

    /**
    * @brief Gets current angles of all motors.
    * @param outAngles Pointer to array to store current angles.
    * @return Error code.
    */
    Error GetCurrentAngles(float* outAngles);

    /**
    * @brief Gets current angles of specified motors.
    * @param ids Array of motor identifiers.
    * @param outAngles Pointer to array to store current angles.
    * @return Error code.
    */
    Error GetCurrentAngles(const Id* ids, float* outAngles, uint8_t count);
}