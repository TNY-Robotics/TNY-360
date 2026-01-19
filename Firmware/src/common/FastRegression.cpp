#include "common/FastRegression.hpp"

FastRegression::FastRegression()
{
    reset();
}

void FastRegression::reset()
{
    n = 0;
    sum_x = 0;
    sum_y = 0;
    sum_x_sq = 0;
    sum_y_sq = 0;
    sum_xy = 0;
}

void FastRegression::addPoint(uint16_t x_val, int y_val)
{
    n++;
    sum_x += x_val;
    sum_y += y_val;
    // Cast to avoid overflow on sums
    sum_x_sq += (int64_t) x_val * x_val;
    sum_xy += (int64_t) x_val * y_val;
    sum_y_sq += (int64_t) y_val * y_val;
}

bool FastRegression::compute(float &slope, float &offset, float &error_std)
{
    if (n < 2) return false;

    // Calculation of the denominator in integer (perfect precision)
    int64_t denom = (n * sum_x_sq) - (sum_x * sum_x);
    
    if (denom == 0) return false; // Division by zero

    // Convert to double only now for the final division
    // The ESP32-S3 has an FPU, float is fast, but double ensures 
    // no precision loss on division of very large numbers.
    
    // Slope (a)
    double a = (double) ((n * sum_xy) - (sum_x * sum_y)) / denom;
    
    // Offset (b)
    double b = (double) (sum_y - (a * sum_x)) / n;

    // Error (RMSE) - Trick: direct calculation from sums
    double sse = sum_y_sq - (b * sum_y) - (a * sum_xy);
    if (sse < 0) sse = 0; // Mathematical safety

    // Fill output variables
    slope = (float) a;
    offset = (float) b;
    error_std = (float) std::sqrt(sse / n);

    return true;
}