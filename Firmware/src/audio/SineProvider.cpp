#include "audio/SineProvider.hpp"
#include "audio/Speaker.hpp"
#include "common/config.hpp"
#include <cmath>

SineProvider::SineProvider()
{
    phase = 0.0f;
    phaseIncrement = (2.0f * M_PI * frequency) / static_cast<float>(SPEAKER_SAMPLE_RATE_HZ);
}

bool SineProvider::provideSamples(Speaker::Sample* buffer, size_t sampleCount)
{
    const float amplitude = 20000.0f * volume; // 30000 est proche de la saturation, 20000 est plus sûr pour tester

    for (size_t i = 0; i < sampleCount; i++)
    {
        buffer[i] = static_cast<Speaker::Sample>(amplitude * std::sin(phase));
        
        phase += phaseIncrement;
        
        // Garder la phase dans [0, 2PI] pour éviter de perdre de la précision float après 1h de fonctionnement
        if (phase > 2.0f * M_PI) {
            phase -= 2.0f * M_PI;
        }
    }

    return true;
}