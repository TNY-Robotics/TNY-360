#pragma once
#include <cmath>

/**
 * @brief RunningStats class for calculating running statistics (min, max, mean, std deviation) in a single pass.
 * @note This class uses Welford's algorithm for numerical stability.
 */
struct RunningStats {
    float min_val = 1e9f;
    float max_val = -1e9f;
    float mean_val = 0.0f;
    float m2 = 0.0f;
    int count = 0;

    void update(float x) {
        count++;
        min_val = std::min(min_val, x);
        max_val = std::max(max_val, x);
        
        float delta = x - mean_val;
        mean_val += delta / count;
        float delta2 = x - mean_val;
        m2 += delta * delta2;
    }

    float std_dev() const {
        return (count > 1) ? std::sqrt(m2 / count) : 0.0f;
    }

    float min() const { return min_val; }
    float max() const { return max_val; }
    float mean() const { return mean_val; }
};