#include "modules/Sound.hpp"

namespace Sound
{
    bool initialized = false;
    float volume = 1.0f; // Volume level (0.0 to 1.0)

    Error Init()
    {
        if (initialized) return Error::Ok;

        // setup hardware (note : using sigma-delta modulation for audio output on a GPIO pin)

        initialized = true;
        return Error::Ok; // TODO
    }

    Error Play(const Note notes[], const float durations_s[], uint8_t count)
    {
        return Error::Ok; // TODO
    }

    Error Play(Note note, float duration_s)
    {
        return Error::Ok; // TODO
    }

    Error Play(const Partition& partition)
    {
        return Play(partition.notes, partition.durations_s, partition.count);
    }

    Error Stop()
    {
        return Error::Ok; // TODO
    }

    bool IsPlaying()
    {
        return false; // TODO
    }

    Error SetVolume(uint8_t volume)
    {
        return Error::Ok; // TODO
    }
}