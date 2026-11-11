#pragma once
#include "common/utils.hpp"
#include "common/geometry.hpp"
#include "locomotion/utils.hpp"

class KinematicsEngine
{
public:
    /// @brief Structure defining the body dimensions for IK and FK computations
    struct KinematicsConfig
    {
        /// @brief X-Distance between body center and hip center
        float hip_shift_x;
        /// @brief Y-Distance between body center and hip center
        float hip_shift_y;
        /// @brief Offset between the hip center and the leg center
        float hip_offset;
        /// @brief Length of the top part of the leg in meters (between the hip and the knee)
        float length_thigh;
        /// @brief Length of the bottom part of the leg in meters (between the knee and the feet)
        float length_calf;
        /// @brief Flag for y-inversion of the legs
        bool leg_inverted[4];
    };

    constexpr static const char* TAG = "KinematicsEngine";

    KinematicsEngine();

    KinematicsEngine(KinematicsConfig config);
    
    /**
     * @brief Computes the full body IK
     * @param cartesian [IN] The cartesian positions / rotations of body parts
     * @param joints [OUT] The joint angles corresponding to the given cartesian state
     * @returns Status::Ok if success, other Error type overwise
     */
    Status computeBodyIK(const BodyCartesianState& cartesian, BodyJointState& joints);
    
    /**
     * @brief Computes the IK for a leg
     * @param target [IN] The target feet position (in hip frame)
     * @param joints [OUT] The joint angles corresponding to the given feet target
     * @returns Status::Ok if success, other Error type overwise
     */
    Status computeLegIK(const Vec3f& target, LegJointState& joints);

private:
    KinematicsConfig config;
};