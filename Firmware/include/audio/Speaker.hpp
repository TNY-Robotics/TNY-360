#pragma once
#include "common/utils.hpp"

/**
 * @brief Base class for speaker modules.
 * A speaker module takes audio samples and outputs them through a speaker (real audio).
 */
class Speaker
{
public:
    constexpr static const char* TAG = "Speaker";

    /**
     * @brief Factory method to create a Speaker instance based on the settings.
     * @return Pointer to the created Speaker instance, or nullptr if disabled or creation failed.
     */
    static Speaker* Create();

    using Sample = int16_t;

    Speaker() = default;

    virtual ~Speaker() = default;

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
