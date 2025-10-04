#include "modules/Sound.hpp"

namespace Sound
{
    static bool initialized = false;
    static float volume = 1.0f; // Volume level (0.0 to 1.0)

    static Error Init()
    {
        if (initialized) return Error::Ok;

        // setup hardware (note : using sigma-delta modulation for audio output on a GPIO pin)

        initialized = true;
        return Error::Ok; // TODO
    }

    static Error Play(const Note notes[], const float durations_s[], uint8_t count)
    {
        return Error::Ok; // TODO
    }

    static Error Play(Note note, float duration_s)
    {
        return Error::Ok; // TODO
    }

    static Error Play(const Partition& partition)
    {
        return Play(partition.notes, partition.durations_s, partition.count);
    }

    static Error Stop()
    {
        return Error::Ok; // TODO
    }

    static bool IsPlaying()
    {
        return false; // TODO
    }

    static Error SetVolume(uint8_t volume)
    {
        return Error::Ok; // TODO
    }
}