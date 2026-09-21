#include "audio/SineProvider.hpp"
#include "audio/ISpeaker.hpp"
#include "common/config.hpp"
#include <cmath>

SineProvider::SineProvider()
{
    phase = 0.0f;
    phaseIncrement = (2.0f * M_PI * frequency) / static_cast<float>(SPEAKER_SAMPLE_RATE_HZ);
}

SineProvider::SineProvider(float frequencyHz, float volume)
    : volume(volume), frequency(frequencyHz)
{
    phase = 0.0f;
    phaseIncrement = (2.0f * M_PI * frequency) / static_cast<float>(SPEAKER_SAMPLE_RATE_HZ);
}

void SineProvider::setFrequency(float frequencyHz)
{
    frequency = frequencyHz;
    phaseIncrement = (2.0f * M_PI * frequency) / static_cast<float>(SPEAKER_SAMPLE_RATE_HZ);
}

void SineProvider::setVolume(float volume)
{
    this->volume = volume;
}

bool SineProvider::provideSamples(ISpeaker::Sample* buffer, size_t sampleCount)
{
    const float amplitude = (INT16_MAX-2) * volume;

    for (size_t i = 0; i < sampleCount; i++)
    {
        buffer[i] = static_cast<ISpeaker::Sample>(amplitude * std::sin(phase));
        
        phase += phaseIncrement;
        
        if (phase > 2.0f * M_PI) {
            phase -= 2.0f * M_PI;
        }
    }

    return true;
}