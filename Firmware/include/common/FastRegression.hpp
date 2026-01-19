#pragma once
#include <cstdint>
#include <cmath>

class FastRegression
{
public:
    FastRegression();

    /**
     * @brief Resets the regression data.
     */
    void reset();

    /**
     * @brief Adds a data point to the regression.
     * @param x_val The x value (independent variable).
     * @param y_val The y value (dependent variable).
     */
    void addPoint(uint16_t x_val, int y_val);

    /**
     * @brief Computes the linear regression parameters.
     * @param slope Reference to store the computed slope.
     * @param offset Reference to store the computed offset.
     * @param error_std Reference to store the standard deviation of the error.
     * @return True if computation was successful, false otherwise.
     */
    bool compute(float &slope, float &offset, float &error_std);

private:
    uint16_t n = 0;        // Point counter
    int64_t sum_x = 0;     // X sum
    int64_t sum_y = 0;     // Y sum
    int64_t sum_x_sq = 0;  // X squared sum
    int64_t sum_y_sq = 0;  // Y squared sum (for standard deviation)
    int64_t sum_xy = 0;    // X*Y product sum
};