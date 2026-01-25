#pragma once
#include "common/utils.hpp"
#include "audio/Speaker.hpp"
#include "audio/SoundMixer.hpp"

class AudioManager
{
public:
    AudioManager();

    /**
     * @brief Initialize the audio manager.
     * @return Error code indicating success or failure.
     */
    Error init();
    
    /**
     * @brief Deinitialize the audio manager.
     * @return Error code indicating success or failure.
     */
    Error deinit();

    /**
     * @brief Get the Speaker instance.
     * @return Reference to the Speaker object.
     */
    Speaker& getSpeaker() { return speaker; }

    /**
     * @brief Get the SoundMixer instance.
     * @return Reference to the SoundMixer object.
     */
    SoundMixer& getMixer() { return mixer; }

private:
    Speaker speaker;
    SoundMixer mixer;
};