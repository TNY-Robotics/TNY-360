#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"

class IMUController
{
public:
    constexpr static const char* TAG = "IMUController";

    IMUController();

    /**
    * @brief Initializes the IMU controller.
    * @return Error code indicating success or failure.
    */
    Error init();

    /**
    * @brief Deinitializes the IMU controller.
    * @return Error code indicating success or failure.
    */
    Error deinit();

    /**
     * @brief Update the IMU controller state.
     * @return Error code indicating success or failure.
     */
    Error update();

    /**
     * @brief Get the down vector representing gravity direction in body frame
     * @return Reference to the down vector.
     */
    Vec3f& getDownVector() { return downVector; }

    /**
     * @brief Get the current orientation estimate
     * @return Reference to the orientation quaternion.
     * @note Yaw is always zeroed, only pitch and roll are represented.
     */
    Quatf& getOrientation() { return orientation; }

private:
    /// @brief Gravity down vector in body frame
    Vec3f downVector;

    /// @brief Current orientation estimate (yaw is always zeroed, only pitch and roll used)
    Quatf orientation;
};