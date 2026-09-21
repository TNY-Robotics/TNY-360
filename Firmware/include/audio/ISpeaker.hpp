#pragma once
#include "common/utils.hpp"
#include "driver/i2s_std.h"

/**
 * @brief Interface for speaker modules.
 * A speaker module takes audio samples and outputs them through a speaker (real audio).
 */
class ISpeaker
{
public:
    constexpr static const char* TAG = "Speaker";

    using Sample = int16_t;

    ISpeaker() = default;

    virtual ~ISpeaker() = default;

    /**
     * @brief Initialize the speaker module.
     * @return Error code indicating success or failure.
     */
    virtual Status init() = 0;

    /**
     * @brief Deinitialize the speaker module.
     * @return Error code indicating success or failure.
     */
    virtual Status deinit() = 0;

    /**
     * @brief Write audio samples to the speaker.
     * @param samples Pointer to the audio samples buffer.
     * @param sampleCount Number of samples to write.
     */
    virtual void writeSamples(const Sample* samples, size_t sampleCount) = 0;
};
