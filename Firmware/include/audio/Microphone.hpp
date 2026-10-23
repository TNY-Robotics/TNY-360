#pragma once
#include "common/utils.hpp"

/**
 * @brief Base class for microphone modules.
 * A microphone module reads audio samples from hardware (real audio).
 */
class Microphone
{
public:
    constexpr static const char* TAG = "Microphone";

    using Sample = int16_t;

    /**
     * @brief Factory method to create a Microphone instance based on the settings.
     * @return Pointer to the created Microphone instance, or nullptr if disabled or creation failed.
     */
    static Microphone* Create();

    Microphone() = default;

    virtual ~Microphone() = default;

    /**
     * @brief Initialize the microphone module.
     * @return Error code indicating success or failure.
     */
    virtual Status init() = 0;

    /**
     * @brief Deinitialize the microphone module.
     * @return Error code indicating success or failure.
     */
    virtual Status deinit() = 0;

    /**
     * @brief Read audio samples from the microphone.
     * @param buffer Pointer to the buffer to fill with audio samples.
     * @param sampleCount Number of samples to read.
     * @return Number of samples actually read.
     */
    virtual size_t readSamples(Sample* buffer, size_t sampleCount) = 0;

    /**
     * @brief Set the gain for the microphone input.
     * @param gain Gain value (1.0 = default gain).
     */
    void setGain(float gain) { this->gain = gain; }

    /**
     * @brief Get the gain of the microphone.
     * @return The gain (1.0 = default gain).
     */
    float getGain() const { return gain; }

    /**
     * @brief Get the sample rate of the microphone.
     * @return The sample rate in Hz.
     */
    int getSampleRate() const { return sampleRate; }

protected:
    int32_t gain = 1;
    uint32_t sampleRate = 16000;
};