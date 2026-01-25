#pragma once

class KalmanFilter1D {
public:
    KalmanFilter1D();

    /**
     * @brief Initializes the Kalman Filter with specified parameters.
     * @param R_variance Variance of the measurement noise (from calibration "Sensor Noise").
     * @param Q_variance Variance of the process noise (Confidence in the model/smoothness).
     * @param initial_value Initial value for the estimate.
     */
    void Init(float R_variance, float Q_variance, float initial_value);

    /**
     * @brief Resets the estimate without changing the R and Q parameters.
     * Useful if the robot is teleported or restarted.
     */
    void ResetState(float value);

    /**
     * @brief Step 1: Prediction (Time Update).
     * Estimates the new position based on the commanded movement.
     * @param control_movement The theoretical requested displacement (cmd_t - cmd_t-1).
     * Set to 0.0f if assuming constant or zero velocity.
     */
    void Predict(float control_movement);

    /**
     * @brief Step 2: Update (Measurement Update).
     * Corrects the prediction with the new actual measurement.
     * @param measurement The raw value read from the sensor (potentiometer).
     * @return The new filtered estimate.
     */
    float Update(float measurement);

    /**
     * @brief Get the current state estimate.
     * @return Current estimated value.
     */
    float GetEstimate() const { return m_x_est; }

    /**
     * @brief Get the current uncertainty (error covariance).
     * @return Current uncertainty value.
     */
    float GetUncertainty() const { return m_P; }

private:
    // State
    float m_x_est; // x : Current estimate
    float m_P;    // P : Current estimate uncertainty (error covariance)

    // Parameters
    float m_Q;     // Q : Process noise covariance (Model)
    float m_R;     // R : Measurement noise covariance (Sensor)
};