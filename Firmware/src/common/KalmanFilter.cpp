#include "common/KalmanFilter.hpp"

KalmanFilter1D::KalmanFilter1D() 
    : _x_est(0.0f), _P(1.0f), _Q(0.1f), _R(1.0f) 
{
}

void KalmanFilter1D::Init(float R_variance, float Q_variance, float initial_value) {
    _R = R_variance;
    _Q = Q_variance;
    ResetState(initial_value);
}

void KalmanFilter1D::ResetState(float value) {
    _x_est = value;
    _P = 1.0f; // Reset uncertainty to a default value
}

void KalmanFilter1D::Predict(float control_movement) {
    // 1. Projection of the state (A priori state estimate)
    // x(k|k-1) = x(k-1|k-1) + u(k)
    _x_est = _x_est + control_movement;

    // 2. Projection of the uncertainty (A priori error covariance)
    // P(k|k-1) = P(k-1|k-1) + Q
    // Uncertainty grows because the theoretical model is never perfect
    _P = _P + _Q;
}

float KalmanFilter1D::Update(float measurement) {
    // 3. Calculation of the Kalman Gain
    // K = P / (P + R)
    // If R is large (noisy sensor), K tends to 0 (trust the model)
    // If P is large (uncertain model), K tends to 1 (trust the measurement)
    float K = _P / (_P + _R);

    // 4. Update the estimate with the measurement (A posteriori state estimate)
    // x(k|k) = x(k|k-1) + K * (z - x(k|k-1))
    _x_est = _x_est + K * (measurement - _x_est);

    // 5. Update the uncertainty (A posteriori error covariance)
    // P(k|k) = (1 - K) * P(k|k-1)
    // Uncertainty decreases because we received sensor information
    _P = (1.0f - K) * _P;

    return _x_est;
}