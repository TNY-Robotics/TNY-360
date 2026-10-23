#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/RPC.hpp"
#include "common/config.hpp"
#include "locomotion/IMU.hpp"
#include "Robot.hpp"
#include <esp_system.h>

namespace Protocol
{
namespace IMU
{
    constexpr uint8_t MODULE_ID = 0x07;

    /** <API_REF>
     * @module imu 0x07
     * @action getAcceleration 0x00
     * @desc Gets the current acceleration vector from the IMU.
     * @result acceleration Vec3f Acceleration vector in m/s^2 (x, y, z).
     * @impl done
     */
    static void GetAcceleration(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<Vec3f>([](){
            ::IMU* imu = Robot::GetInstance().getBody().getIMU();
            if (!imu) return Vec3f(0, 0, 0);
            return imu->getAcceleration();
        }, [ctx](Vec3f vec){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &vec, sizeof(vec));
        });
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getAngularVelocity 0x01
     * @desc Gets the current angular velocity vector from the IMU.
     * @result angular_velocity Vec3f Angular velocity vector in rad/s (x, y, z).
     * @impl done
     */
    static void GetAngularVelocity(const RequestContext& ctx, const uint8_t* payload)
    {
        RPC::ExecuteThreadSafe<Vec3f>([](){
            ::IMU* imu = Robot::GetInstance().getBody().getIMU();
            if (!imu) return Vec3f(0, 0, 0);
            return imu->getAngularVelocity();
        }, [ctx](Vec3f vec){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &vec, sizeof(vec));
        });
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getOrientation 0x02
     * @desc Gets the current orientation of the robot as a rotation vector (axis-angle representation).
     * @result orientation Quatf Rotation quaternion representing the robot's orientation.
      * @impl done
     */
    static void GetOrientation(const RequestContext& ctx, const uint8_t* payload)
    {
        const Quatf& orient = Robot::GetInstance().getDecisionLoop().getRobotState().body_orientation;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &orient, sizeof(orient));
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getCalibrationState 0x03
     * @desc Gets the current calibration state of the IMU.
     * @result state uint8 Calibration state (0 = not calibrated, 1 = calibrating, 2 = calibrated).
      * @impl nope
     */
    static void GetCalibrationState(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getCalibrationData 0x04
     * @desc Gets the current calibration data of the IMU.
     * @result data byte[] Calibration data blob (format TBD).
      * @impl nope
     */
    static void GetCalibrationData(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
    * @module imu 0x07
    * @action setCalibrationData 0x05
    * @desc Sets the calibration data of the IMU.
    * @arg data byte[] Calibration data blob (format TBD).
    * @impl nope
    */
    static void SetCalibrationData(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
     * @module imu 0x07
     * @action startCalibration 0x06
     * @desc Starts the calibration process for the IMU.
     * @impl nope
     */
    static void StartCalibration(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
     * @module imu 0x07
     * @action stopCalibration 0x07
     * @desc Stops the calibration process for the IMU.
     * @impl nope
     */
    static void StopCalibration(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getCalibrationProgress 0x08
     * @desc Gets the current progress of the IMU calibration process.
     * @result progress uint8 Calibration progress percentage (0-100).
     * @impl nope
     */
    static void GetCalibrationProgress(const RequestContext& ctx, const uint8_t* payload)
    {
        ctx.respond(ResponseStatus::Ok);
        // TODO : Implement
    }


    static ActionCallback actions[] = {
        GetAcceleration,        // 0x00
        GetAngularVelocity,     // 0x01
        GetOrientation,         // 0x02
        GetCalibrationState,    // 0x03
        GetCalibrationData,     // 0x04
        SetCalibrationData,     // 0x05
        StartCalibration,       // 0x06
        StopCalibration,        // 0x07
        GetCalibrationProgress, // 0x08
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}