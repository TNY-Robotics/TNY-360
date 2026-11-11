#include "audio/SynthProvider.hpp"
#include "settings/Settings.hpp"
#include <cmath>
#include <algorithm>

// Sin wave lookup table for 256 samples (0-255)
static constexpr int16_t SINE_LUT[256] = {
        0,   804,  1607,  2410,  3211,  4011,  4807,  5601,
     6392,  7179,  7961,  8739,  9511, 10278, 11038, 11792,
    12539, 13278, 14009, 14732, 15446, 16150, 16845, 17530,
    18204, 18867, 19519, 20159, 20787, 21402, 22004, 22594,
    23169, 23731, 24278, 24811, 25329, 25831, 26318, 26789,
    27244, 27683, 28105, 28510, 28897, 29268, 29621, 29955,
    30272, 30571, 30851, 31113, 31356, 31580, 31785, 31970,
    32137, 32284, 32412, 32520, 32609, 32678, 32727, 32757,
    32767, 32757, 32727, 32678, 32609, 32520, 32412, 32284,
    32137, 31970, 31785, 31580, 31356, 31113, 30851, 30571,
    30272, 29955, 29621, 29268, 28897, 28510, 28105, 27683,
    27244, 26789, 26318, 25831, 25329, 24811, 24278, 23731,
    23169, 22594, 22004, 21402, 20787, 20159, 19519, 18867,
    18204, 17530, 16845, 16150, 15446, 14732, 14009, 13278,
    12539, 11792, 11038, 10278,  9511,  8739,  7961,  7179,
     6392,  5601,  4807,  4011,  3211,  2410,  1607,   804,
        0,  -804, -1607, -2410, -3211, -4011, -4807, -5601,
    -6392, -7179, -7961, -8739, -9511,-10278,-11038,-11792,
   -12539,-13278,-14009,-14732,-15446,-16150,-16845,-17530,
   -18204,-18867,-19519,-20159,-20787,-21402,-22004,-22594,
   -23169,-23731,-24278,-24811,-25329,-25831,-26318,-26789,
   -27244,-27683,-28105,-28510,-28897,-29268,-29621,-29955,
   -30272,-30571,-30851,-31113,-31356,-31580,-31785,-31970,
   -32137,-32284,-32412,-32520,-32609,-32678,-32727,-32757,
   -32767,-32757,-32727,-32678,-32609,-32520,-32412,-32284,
   -32137,-31970,-31785,-31580,-31356,-31113,-30851,-30571,
   -30272,-29955,-29621,-29268,-28897,-28510,-28105,-27683,
   -27244,-26789,-26318,-25831,-25329,-24811,-24278,-23731,
   -23169,-22594,-22004,-21402,-20787,-20159,-19519,-18867,
   -18204,-17530,-16845,-16150,-15446,-14732,-14009,-13278,
   -12539,-11792,-11038,-10278, -9511, -8739, -7961, -7179,
    -6392, -5601, -4807, -4011, -3211, -2410, -1607,  -804
};

SynthProvider::SynthProvider(Partition partition)
    : m_partition(std::move(partition)),
      m_sampleRate(Settings::GetConfig().audio.speaker.sampleRate)
{
    if (m_sampleRate == 0) m_sampleRate = 44100;
    setCutoffFrequency(4000.0f);
    prepareNextNote();
}

SynthProvider::SynthProvider(const Note& singleNote)
    : m_partition({singleNote}),
      m_sampleRate(Settings::GetConfig().audio.speaker.sampleRate)
{
    if (m_sampleRate == 0) m_sampleRate = 44100;
    setCutoffFrequency(4000.0f);
    prepareNextNote();
}

void SynthProvider::setCutoffFrequency(float cutoffHz)
{
    if (cutoffHz <= 0.0f || m_sampleRate == 0)
    {
        m_lpfAlpha = 1.0f;
        return;
    }
    float dt = 1.0f / static_cast<float>(m_sampleRate);
    float rc = 1.0f / (2.0f * static_cast<float>(M_PI) * cutoffHz);
    m_lpfAlpha = std::clamp(dt / (rc + dt), 0.01f, 1.0f);
}

void SynthProvider::prepareNextNote()
{
    if (m_currentNoteIndex >= m_partition.size())
    {
        m_samplesRemainingInNote = 0;
        return;
    }

    const Note& note = m_partition.data()[m_currentNoteIndex];
    m_samplesRemainingInNote = (static_cast<uint64_t>(note.durationMs) * m_sampleRate) / 1000;
    
    // Auto-release de fin de note (3 ms)
    uint32_t releaseMs = std::min(3lu, note.durationMs / 4);
    m_releaseSamplesTotal = (static_cast<uint64_t>(releaseMs) * m_sampleRate) / 1000;

    // Targets
    m_targetFreq = note.frequency;
    m_targetVolume = std::clamp(note.volume, 0.0f, 1.0f);

    // Si c'est la toute première note, on cale la fréquence tout de suite
    if (m_currentNoteIndex == 0)
    {
        m_currentFreq = m_targetFreq;
    }

    // --- CORRECTION CLÉ DU TIC ---
    // À la fin de la note précédente, le son a été coupé à 0 par l'auto-release.
    // L'attaque de la nouvelle note DOIT donc impérativement repartir de 0.0f !
    m_currentVolume = 0.0f;

    // Glide config
    m_glideSamplesRemaining = (static_cast<uint64_t>(note.glideTimeMs) * m_sampleRate) / 1000;
    if (m_glideSamplesRemaining > 0)
    {
        m_freqStepPerSample = (m_targetFreq - m_currentFreq) / static_cast<float>(m_glideSamplesRemaining);
    } 
    else
    {
        m_currentFreq = m_targetFreq;
        m_freqStepPerSample = 0.0f;
    }

    // Fade config (Attaque en volume depuis 0.0f vers targetVolume)
    m_fadeSamplesRemaining = (static_cast<uint64_t>(note.fadeTimeMs) * m_sampleRate) / 1000;
    if (m_fadeSamplesRemaining > 0)
    {
        m_volStepPerSample = (m_targetVolume - 0.0f) / static_cast<float>(m_fadeSamplesRemaining);
    } 
    else
    {
        m_currentVolume = m_targetVolume;
        m_volStepPerSample = 0.0f;
    }
}

Speaker::Sample SynthProvider::generateSample()
{
    const Note& note = m_partition.data()[m_currentNoteIndex];
    
    // 1. Glide interpolation
    if (m_glideSamplesRemaining > 0) {
        m_currentFreq += m_freqStepPerSample;
        --m_glideSamplesRemaining;
    } else {
        m_currentFreq = m_targetFreq;
    }

    // 2. Fade interpolation (Attaque)
    if (m_fadeSamplesRemaining > 0) {
        m_currentVolume += m_volStepPerSample;
        --m_fadeSamplesRemaining;
    } else {
        m_currentVolume = m_targetVolume;
    }

    // 3. Auto-release en fin de note (Ramène doucement à zéro sur les 3 dernières ms)
    float releaseFactor = 1.0f;
    if (m_samplesRemainingInNote <= m_releaseSamplesTotal && m_releaseSamplesTotal > 0) {
        releaseFactor = static_cast<float>(m_samplesRemainingInNote) / static_cast<float>(m_releaseSamplesTotal);
    }

    // 4. LFO (Tremolo d'amplitude + Vibrato de pitch)
    float tremoloFactor = 1.0f;
    float effectiveFreq = m_currentFreq;

    if (note.tremoloSpeedHz > 0.0f && note.tremoloDepth > 0.0f)
    {
        uint8_t lfoLutIndex = static_cast<uint8_t>(m_lfoPhase * 256.0f);
        float lfoSin = SINE_LUT[lfoLutIndex] / 32767.0f; // [-1.0f, 1.0f]

        // On centre le tremolo autour de 1.0 avec une modulation symétrique plus douce
        tremoloFactor = 1.0f - (note.tremoloDepth * 0.5f * (1.0f + lfoSin));

        // Vibrato (max 4% pour ne pas détonner)
        constexpr float kVibratoScale = 0.04f;
        effectiveFreq = m_currentFreq * (1.0f + (note.tremoloDepth * kVibratoScale * lfoSin));

        m_lfoPhase += note.tremoloSpeedHz / static_cast<float>(m_sampleRate);
        if (m_lfoPhase >= 1.0f) {
            m_lfoPhase -= std::floor(m_lfoPhase);
        }
    }

    // 5. Waveform generation
    float rawValue = 0.0f;
    switch (note.waveform)
    {
        case Waveform::Sine: {
            uint8_t lutIndex = static_cast<uint8_t>(m_phase * 256.0f);
            rawValue = SINE_LUT[lutIndex] / 32767.0f;
            break;
        }
        case Waveform::Triangle: {
            rawValue = (m_phase < 0.5f) ? (4.0f * m_phase - 1.0f) : (3.0f - 4.0f * m_phase);
            break;
        }
        case Waveform::Square: {
            rawValue = (m_phase < note.dutyCycle) ? 1.0f : -1.0f;
            break;
        }
        case Waveform::Noise: {
            m_noiseRng = m_noiseRng * 1664525u + 1013904223u;
            rawValue = static_cast<int16_t>(m_noiseRng >> 16) / 32768.0f;
            break;
        }
        case Waveform::Silence:
        default:
            rawValue = 0.0f;
            break;
    }

    // 6. Update phase
    if (note.waveform != Waveform::Silence && note.waveform != Waveform::Noise)
    {
        float phaseInc = effectiveFreq / static_cast<float>(m_sampleRate);
        m_phase += phaseInc;
        if (m_phase >= 1.0f) {
            m_phase -= std::floor(m_phase);
        }
    }

    // 7. Sortie finale : Amplitude * Tremolo * Fade d'extinction
    float sampleOut = rawValue * (m_currentVolume * tremoloFactor * releaseFactor);

    // 8. Low-pass filter (adoucissement)
    m_lpfPrevSample += m_lpfAlpha * (sampleOut - m_lpfPrevSample);
    
    return static_cast<Speaker::Sample>(std::clamp(
        m_lpfPrevSample * 32767.0f,
        -32768.0f,
        32767.0f
    ));
}

bool SynthProvider::provideSamples(Speaker::Sample* buffer, size_t sampleCount)
{
    if (m_partition.empty())
    {
        return false;
    }

    for (size_t i = 0; i < sampleCount; ++i)
    {
        while (m_samplesRemainingInNote == 0)
        {
            ++m_currentNoteIndex;
            if (m_currentNoteIndex >= m_partition.size())
            {
                constexpr size_t kDecaySamples = 64;
                size_t samplesToFade = std::min(kDecaySamples, sampleCount - i);

                for (size_t j = 0; j < samplesToFade; ++j)
                {
                    m_lpfPrevSample += m_lpfAlpha * (0.0f - m_lpfPrevSample);
                    buffer[i + j] = static_cast<Speaker::Sample>(
                        std::clamp(
                            m_lpfPrevSample * 32767.0f, -32768.0f, 32767.0f)
                    );
                }

                if (i + samplesToFade < sampleCount)
                {
                    std::fill(buffer + i + samplesToFade, buffer + sampleCount, static_cast<Speaker::Sample>(0));
                }

                m_lpfPrevSample = 0.0f;
                return false;
            }
            prepareNextNote();
        }

        buffer[i] = generateSample();
        --m_samplesRemainingInNote;
    }

    return true;
}