#pragma once
#include "network/protocol/Protocol.hpp"
#include "common/RPC.hpp"
#include "common/config.hpp"
#include "drivers/IMUDriver.hpp"
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
            IMUDriver::IMUData data = IMUDriver::GetData();
            return Vec3f(data.accel_x_g * GRAVITY, data.accel_y_g * GRAVITY, data.accel_z_g * GRAVITY);
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
            IMUDriver::IMUData data = IMUDriver::GetData();
            return Vec3f(DEG_TO_RAD(data.gyro_x_ds), DEG_TO_RAD(data.gyro_y_ds), DEG_TO_RAD(data.gyro_z_ds));
        }, [ctx](Vec3f vec){
            ctx.respond(ResponseStatus::Ok, (uint8_t*) &vec, sizeof(vec));
        });
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getDownVector 0x02
     * @desc Gets the current down vector from the IMU (i.e. the direction of gravity).
     * @result down_vector Vec3f Down vector (unit vector pointing in the direction of gravity).
      * @impl done
     */
    static void GetDownVector(const RequestContext& ctx, const uint8_t* payload)
    {
        const Vec3f& vec = Robot::GetInstance().getDecisionLoop().getRobotState().imu_down_vector;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &vec, sizeof(vec));
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getOrientation 0x03
     * @desc Gets the current orientation of the robot as a rotation vector (axis-angle representation).
     * @result orientation Vec3f Rotation vector representing the robot's orientation (Euler rotation in radians, XYZ order)
     * @impl done
     */
    static void GetOrientation(const RequestContext& ctx, const uint8_t* payload)
    {
        const Vec3f& orient = Robot::GetInstance().getDecisionLoop().getRobotState().body_orientation;
        ctx.respond(ResponseStatus::Ok, (uint8_t*) &orient, sizeof(orient));
    }

    /** <API_REF>
     * @module imu 0x07
     * @action getCalibrationState 0x04
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
     * @action getCalibrationData 0x05
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
    * @action setCalibrationData 0x06
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
     * @action startCalibration 0x07
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
     * @action stopCalibration 0x08
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
     * @action getCalibrationProgress 0x09
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
        GetAcceleration,           // 0x00
        GetAngularVelocity,        // 0x01
        GetDownVector,             // 0x02
        GetOrientation,            // 0x03
        GetCalibrationState,       // 0x04
        GetCalibrationData,        // 0x05
        SetCalibrationData,        // 0x06
        StartCalibration,          // 0x07
        StopCalibration,           // 0x08
        GetCalibrationProgress,    // 0x09
    };

    static void Register(Dispatcher& dispatcher)
    {
        dispatcher.registerModule(MODULE_ID, actions, sizeof(actions));
    }
}
}