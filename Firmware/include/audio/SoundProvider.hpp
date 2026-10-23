#pragma once
#include "common/utils.hpp"
#include "audio/Speaker.hpp"

class SoundProvider
{
public:
    SoundProvider() = default;
    virtual ~SoundProvider() = default;

    /**
     * @brief Provide audio samples to fill the buffer.
     * @param buffer Pointer to the buffer to fill with audio samples.
     * @param sampleCount Number of samples to provide.
     * @return true if samples were provided, false if no more samples are available.
     * @note If false is returned, the mixer will delete this provider from its list.
     */
    virtual bool provideSamples(Speaker::Sample* buffer, size_t sampleCount) = 0;
};