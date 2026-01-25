#include "common/KalmanFilter.hpp"

KalmanFilter1D::KalmanFilter1D() 
    : m_x_est(0.0f), m_P(1.0f), m_Q(0.1f), m_R(1.0f) 
{
}

void KalmanFilter1D::Init(float R_variance, float Q_variance, float initial_value) {
    m_R = R_variance;
    m_Q = Q_variance;
    ResetState(initial_value);
}

void KalmanFilter1D::ResetState(float value) {
    m_x_est = value;
    m_P = 1.0f; // Reset uncertainty to a default value
}

void KalmanFilter1D::Predict(float control_movement) {
    // 1. Projection of the state (A priori state estimate)
    // x(k|k-1) = x(k-1|k-1) + u(k)
    m_x_est = m_x_est + control_movement;

    // 2. Projection of the uncertainty (A priori error covariance)
    // P(k|k-1) = P(k-1|k-1) + Q
    // Uncertainty grows because the theoretical model is never perfect
    m_P = m_P + m_Q;
}

float KalmanFilter1D::Update(float measurement) {
    // 3. Calculation of the Kalman Gain
    // K = P / (P + R)
    // If R is large (noisy sensor), K tends to 0 (trust the model)
    // If P is large (uncertain model), K tends to 1 (trust the measurement)
    float K = m_P / (m_P + m_R);

    // 4. Update the estimate with the measurement (A posteriori state estimate)
    // x(k|k) = x(k|k-1) + K * (z - x(k|k-1))
    m_x_est = m_x_est + K * (measurement - m_x_est);

    // 5. Update the uncertainty (A posteriori error covariance)
    // P(k|k) = (1 - K) * P(k|k-1)
    // Uncertainty decreases because we received sensor information
    m_P = (1.0f - K) * m_P;

    return m_x_est;
}