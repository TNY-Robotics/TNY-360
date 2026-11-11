#pragma once

#include "SoundProvider.hpp"
#include "SynthDefinitions.hpp"

class SynthProvider : public SoundProvider {
public:
    explicit SynthProvider(Partition partition);
    explicit SynthProvider(const Note& singleNote);
    ~SynthProvider() override = default;

    bool provideSamples(Speaker::Sample* buffer, size_t sampleCount) override;

private:
    void setCutoffFrequency(float cutoffHz);
    void prepareNextNote();
    Speaker::Sample generateSample();

    Partition m_partition;
    size_t    m_currentNoteIndex = 0;
    
    // Sample rate, read from settings or default to 44100 Hz
    uint32_t  m_sampleRate = 44100;

    // Current note state
    uint32_t  m_samplesRemainingInNote = 0;

    // Current LFO state
    float m_lfoPhase = 0.0f;
    
    // Frequency interpolation (Glide/Portamento)
    float     m_currentFreq = 0.0f;
    float     m_targetFreq = 0.0f;
    float     m_freqStepPerSample = 0.0f;
    uint32_t  m_glideSamplesRemaining = 0;

    // Amplitude interpolation (Fade In/Out)
    float     m_currentVolume = 0.0f;
    float     m_targetVolume = 0.0f;
    float     m_volStepPerSample = 0.0f;
    uint32_t  m_fadeSamplesRemaining = 0;

    // Oscillator state
    float     m_phase = 0.0f; // Normalized [0.0, 1.0[ 
    uint32_t  m_noiseRng = 0x12345678; // LCG for noise generation

    // low pass filter 
    float m_lpfAlpha = 0.35f;
    float m_lpfPrevSample = 0.0f;

    // Sample release
    uint32_t m_releaseSamplesTotal = 0;
    uint32_t m_releaseSamplesRemaining = 0;
};