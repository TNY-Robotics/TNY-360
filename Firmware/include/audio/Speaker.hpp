#pragma once
#include "common/utils.hpp"

class Speaker
{
public:
    constexpr static const char* TAG = "Speaker";

    enum class Volume : uint8_t {
        Mute = 0,
        Low = 3,
        Medium = 6,
        High = 10
    };

    using Sample = int16_t;

    Speaker();

    /**
     * @brief Initialize the speaker module.
     * @return Error code indicating success or failure.
     */
    Error init();

    /**
     * @brief Deinitialize the speaker module.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Write audio samples to the speaker.
     * @param samples Pointer to the audio samples buffer.
     * @param sampleCount Number of samples to write.
     */
    void writeSamples(const Sample* samples, size_t sampleCount);
};
