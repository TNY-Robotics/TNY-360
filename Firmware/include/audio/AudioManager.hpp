#pragma once
#include "common/utils.hpp"
#include "audio/Speaker.hpp"
#include "audio/Microphone.hpp"
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
     * @return Pointer to the Speaker object (null if disabled).
     */
    Speaker* getSpeaker() { return speaker; }

    /**
     * @brief Get the Microphone instance.
     * @return Pointer to the Microphone object (null if disabled).
     */
    Microphone* getMicrophone() { return microphone; }

    /**
     * @brief Get the SoundMixer instance.
     * @return Reference to the SoundMixer object.
     */
    SoundMixer& getMixer() { return mixer; }

private:
    Speaker* speaker;
    Microphone* microphone;
    SoundMixer mixer;
};