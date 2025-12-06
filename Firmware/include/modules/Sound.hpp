#pragma once
#include "utils.hpp"

namespace Sound
{
    namespace Freq
    {
        // Octave 3 (Bass)
        constexpr float C_3  = 130.81f; // Do
        constexpr float C_S_3 = 138.59f; // Do#
        constexpr float D_3  = 146.83f; // Re
        constexpr float D_S_3 = 155.56f; // Re#
        constexpr float E_3  = 164.81f; // Mi
        constexpr float F_3  = 174.61f; // Fa
        constexpr float F_S_3 = 185.00f; // Fa#
        constexpr float G_3  = 196.00f; // Sol
        constexpr float G_S_3 = 207.65f; // Sol#
        constexpr float A_3  = 220.00f; // La
        constexpr float A_S_3 = 233.08f; // La#
        constexpr float B_3  = 246.94f; // Si

        // Octave 4 (Middle / Standard)
        constexpr float C_4  = 261.63f; // Middle C
        constexpr float C_S_4 = 277.18f;
        constexpr float D_4  = 293.66f;
        constexpr float D_S_4 = 311.13f;
        constexpr float E_4  = 329.63f; 
        constexpr float F_4  = 349.23f;
        constexpr float F_S_4 = 369.99f; 
        constexpr float G_4  = 392.00f; 
        constexpr float G_S_4 = 415.30f;
        constexpr float A_4  = 440.00f;
        constexpr float A_S_4 = 466.16f;
        constexpr float B_4  = 493.88f; 

        // Octave 5 (High)
        constexpr float C_5  = 523.25f; 
        constexpr float C_S_5 = 554.37f;
        constexpr float D_5  = 587.33f; 
        constexpr float D_S_5 = 622.25f;
        constexpr float E_5  = 659.25f; 
        constexpr float F_5  = 698.46f; 
        constexpr float F_S_5 = 739.99f;
        constexpr float G_5  = 783.99f; 
        constexpr float G_S_5 = 830.61f;
        constexpr float A_5  = 880.00f; 
        constexpr float A_S_5 = 932.33f;
        constexpr float B_5  = 987.77f;
    }

    typedef struct Note
    {
        float frequency; // Frequency in Hz
        float duration_s; // Duration in seconds
        float volume;    // Volume level (0.0 to 1.0)
    } Note;

    typedef struct Partition
    {
        const Note* notes;
        uint8_t count;
    } Partition;

    /**
     * @brief Initializes the Sound module.
     * @return Error code indicating success or failure.
     */
    Error Init();

    /**
     * @brief Plays a single note for a specified duration.
     * @param note The note to play.
     * @param duration_s Duration (in seconds) to play the note.
     * @return Error code indicating success or failure.
     */
    Error Play(const Note& note);

    /**
     * @brief Plays a partition (sequence of notes and durations).
     * @param partition The partition to play.
     * @return Error code indicating success or failure.
     */
    Error Play(const Partition& partition);

    /**
     * @brief Stops any currently playing sound.
     * @return Error code indicating success or failure.
     */
    Error Stop();

    /**
     * @brief Checks if a sound is currently playing.
     * @return True if playing, false otherwise.
     */
    bool IsPlaying();

    /**
     * @brief Sets the volume level.
     * @param volume Volume level (0.0 to 1.0).
     * @return Error code indicating success or failure.
     */
    Error SetVolume(float volume);

    inline Note note(float frequency, float duration_s, float volume = 1.0f)
    {
        return Note{frequency, duration_s, volume};
    }

    inline Note pause(float duration_s)
    {
        return Note{0.0f, duration_s, 0.0f};
    }
}