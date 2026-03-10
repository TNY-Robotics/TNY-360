#pragma once
#include "audio/SoundProvider.hpp"
#include "common/utils.hpp"

class SineProvider : public SoundProvider
{
public:
    SineProvider();

    SineProvider(float frequencyHz, float volume);

    bool provideSamples(Speaker::Sample* buffer, size_t sampleCount) override;

private:
    float volume = 1.0f;
    float frequency = 440.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
};