#pragma once
#include "audio/ISpeaker.hpp"
#include "audio/SoundProvider.hpp"
#include "common/config.hpp"
#include <mutex>

class SoundMixer
{
public:
    constexpr static const char* TAG = "SoundMixer";

    SoundMixer();

    Status init(ISpeaker* speaker);

    Status deinit();

    void setVolume(float volume);

    /**
     * @brief Add an audio provider to the mixer
     * @param provider Pointer to the audio provider to add
     * @return Error code indicating success or failure
     * @note The ownership of the provider is transferred to the mixer, which will delete it when no longer needed
     */
    Status addSoundProvider(SoundProvider* provider);

    /**
     * @brief Internal mixing task
     * @note YOU SHOULD NOT CALL THIS FUNCTION DIRECTLY
     * @param pvParams Pointer to task parameters
     */
    void __internal_task(void* pvParams);

private:
    constexpr static size_t MIX_BUFFER_SIZE = 512;

    SoundProvider* providers[SPEAKER_NB_AUDIO_PROVIDERS];
    std::mutex mixerMutex;
    float masterVolume; // Master volume (0.0 to 1.0)
    ISpeaker* speaker;
    bool running;

    ISpeaker::Sample mixBuffer[MIX_BUFFER_SIZE];
    ISpeaker::Sample sourceBuffer[MIX_BUFFER_SIZE];
};