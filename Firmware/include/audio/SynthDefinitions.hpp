#pragma once
#include "common/utils.hpp"
#include <vector>
#include <span>
#include <initializer_list>

enum class Waveform : uint8_t {
    Sine,
    Triangle,
    Square,
    Noise,
    Silence
};

struct Note {
    Waveform waveform    = Waveform::Square; // Waveform type
    float    dutyCycle   = 0.5f;   // From 0.05f to 0.95f (square wave)
    float    frequency   = 440.0f; // Frequency in Hz
    uint32_t glideTimeMs = 0;      // Portamento to this frequency
    uint32_t fadeTimeMs  = 5;      // Time to target volume
    float    volume      = 1.0f;   // From 0.0f to 1.0f
    uint32_t durationMs  = 100;    // Duration of the note in milliseconds
    float    tremoloSpeedHz   = 0.0f; // 0 = disabled, >0 = speed of tremolo in Hz
    float    tremoloDepth     = 0.0f; // 0.0f (none) to 1.0f (complete silence)

    // --- Fluent API Helpers ---
    constexpr Note& setWaveform(Waveform w)  { waveform = w; return *this; }
    constexpr Note& setDuty(float d)        { dutyCycle = d; return *this; }
    constexpr Note& setFrequency(float f)   { frequency = f; return *this; }
    constexpr Note& setGlide(uint32_t ms)   { glideTimeMs = ms; return *this; }
    constexpr Note& setFade(uint32_t ms)    { fadeTimeMs = ms; return *this; }
    constexpr Note& setVolume(float v)      { volume = v; return *this; }
    constexpr Note& setDuration(uint32_t ms) { durationMs = ms; return *this; }
    constexpr Note& setTremolo(float speedHz, float depth) { tremoloSpeedHz = speedHz; tremoloDepth = depth; return *this; }

    static constexpr Note Rest(uint32_t durationMs) {
        Note n;
        n.waveform = Waveform::Silence;
        n.volume = 0.0f;
        n.durationMs = durationMs;
        return n;
    }

    static constexpr Note Tone(float freq, uint32_t durationMs, Waveform wave = Waveform::Square) {
        Note n;
        n.frequency = freq;
        n.durationMs = durationMs;
        n.waveform = wave;
        return n;
    }
};

class Partition {
public:
    Partition() = default;
    Partition(std::initializer_list<Note> notes) : m_dynamicNotes(notes) {}
    Partition(const Note* notes, size_t count) : m_flashView(notes, count) {}

    Partition& add(const Note& note) {
        m_dynamicNotes.push_back(note);
        return *this;
    }

    const Note* data() const {
        return m_flashView.empty() ? m_dynamicNotes.data() : m_flashView.data();
    }

    size_t size() const {
        return m_flashView.empty() ? m_dynamicNotes.size() : m_flashView.size();
    }

    bool empty() const {
        return size() == 0;
    }

private:
    std::vector<Note> m_dynamicNotes;
    std::span<const Note> m_flashView;
};

// Common note frequencies
namespace Freq
{
    constexpr float C3 = 130.81f, D3 = 146.83f, E3 = 164.81f, F3 = 174.61f, G3 = 196.00f, A3 = 220.00f, B3 = 246.94f;
    constexpr float C4 = 261.63f, D4 = 293.66f, E4 = 329.63f, F4 = 349.23f, G4 = 392.00f, A4 = 440.00f, B4 = 493.88f;
    constexpr float C5 = 523.25f, D5 = 587.33f, E5 = 659.25f, F5 = 698.46f, G5 = 783.99f, A5 = 880.00f, B5 = 987.77f;
    constexpr float C6 = 1046.50f, D6 = 1174.66f, E6 = 1318.51f, G6 = 1567.98f;
}

// Default jingles for robot behavior
namespace Jingle
{
    static Partition Boot = Partition((Note[]){
        Note::Tone(320.f, 35, Waveform::Square).setDuty(0.4f).setGlide(25).setFade(4).setVolume(0.8f),
        Note::Rest(50),
        Note::Tone(320.f, 35, Waveform::Square).setDuty(0.1f).setGlide(25).setFade(4).setVolume(0.9f),
        Note::Rest(80),
    }, 4);
    
    static Partition Success = Partition((Note[]){
        Note::Tone(Freq::C5, 85, Waveform::Triangle).setFade(8),
        Note::Tone(Freq::E5, 85, Waveform::Triangle).setFade(8),
        Note::Tone(Freq::G5, 200, Waveform::Triangle).setFade(20),
    }, 3);

    static Partition Error = Partition((Note[]){
        Note::Tone(Freq::C5, 85, Waveform::Triangle).setFade(8),
        Note::Tone(Freq::G4, 85, Waveform::Triangle).setFade(8),
        Note::Tone(Freq::G4, 200, Waveform::Triangle).setFade(20),
    }, 3);
}