#pragma once
#include "utils.hpp"

namespace Sound
{
    typedef struct Note
    {
        
    } Note;

    typedef struct Partition
    {
        const Note* notes;
        const float* durations_s;
        uint8_t count;
    } Partition;

    /**
     * @brief Initializes the Sound module.
     * @return Error code indicating success or failure.
     */
    Error Init();

    /**
     * @brief Plays a sequence of notes with specified durations.
     * @param notes Array of notes to play.
     * @param durations_s Array of durations (in seconds) for each note.
     * @param count Number of notes in the sequence.
     * @return Error code indicating success or failure.
     */
    Error Play(const Note notes[], const float durations_s[], uint8_t count);

    /**
     * @brief Plays a single note for a specified duration.
     * @param note The note to play.
     * @param duration_s Duration (in seconds) to play the note.
     * @return Error code indicating success or failure.
     */
    Error Play(Note note, float duration_s);

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
}