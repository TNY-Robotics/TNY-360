#pragma once
#include "audio/Speaker.hpp"
#include "audio/SoundProvider.hpp"
#include "common/config.hpp"
#include <mutex>

class SoundMixer
{
public:
    constexpr static const char* TAG = "SoundMixer";

    SoundMixer(Speaker& speaker);

    Error init();

    Error deinit();

    void setVolume(float volume);

    /**
     * @brief Add an audio provider to the mixer
     * @param provider Pointer to the audio provider to add
     * @return Error code indicating success or failure
     * @note The ownership of the provider is transferred to the mixer, which will delete it when no longer needed
     */
    Error addSoundProvider(SoundProvider* provider);

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
    Speaker& speaker;
    bool running;

    Speaker::Sample mixBuffer[MIX_BUFFER_SIZE];
    Speaker::Sample sourceBuffer[MIX_BUFFER_SIZE];
};