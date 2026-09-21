#pragma once
#include "common/utils.hpp"
#include "audio/ISpeaker.hpp"
#include "audio/SoundMixer.hpp"

class AudioManager
{
public:
    constexpr static const char* TAG = "AudioManager";

    AudioManager();

    /**
     * @brief Initialize the audio manager.
     * @return Error code indicating success or failure.
     */
    Status init();
    
    /**
     * @brief Deinitialize the audio manager.
     * @return Error code indicating success or failure.
     */
    Status deinit();

    /**
     * @brief Get the Speaker instance.
     * @return Reference to the Speaker object.
     */
    ISpeaker& getSpeaker() { return *speaker; }

    /**
     * @brief Get the SoundMixer instance.
     * @return Reference to the SoundMixer object.
     */
    SoundMixer& getMixer() { return mixer; }

private:
    ISpeaker* speaker;
    SoundMixer mixer;
};