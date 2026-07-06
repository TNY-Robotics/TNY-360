#include "audio/SoundMixer.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <memory.h>
#include "common/Log.hpp"
#include "common/Error.hpp"
#include "audio/Speaker.Error.hpp"

SoundMixer::SoundMixer(Speaker& speaker) : speaker(speaker)
{
}

Status SoundMixer::init()
{
    std::lock_guard<std::mutex> lock(mixerMutex);
    for (size_t i = 0; i < SPEAKER_NB_AUDIO_PROVIDERS; i++)
    {
        providers[i] = nullptr;
    }
    
    masterVolume = 0.05f; // default volume
    running = true; // start running

    BaseType_t ret = xTaskCreatePinnedToCore([](void* pvParams) {
        SoundMixer* mixer = static_cast<SoundMixer*>(pvParams);
        mixer->__internal_task(nullptr);
    }, "SoundMixerTask", 4096, this, tskIDLE_PRIORITY + 20, nullptr, CORE_BRAIN); // High priority for audio task

    if (ret != pdPASS)
    {
        LOG_ERROR(TAG, "Failed to create SoundMixer task");
        Error::RegisterErrorEvent(ErrorEventCreateTaskFailed(ret));
        return Status::Failure;
    }

    return Status::Ok;
}

Status SoundMixer::deinit()
{
    std::lock_guard<std::mutex> lock(mixerMutex);
    running = false; // ask task to stop
    // delete remaining audio providers
    for (size_t i = 0; i < SPEAKER_NB_AUDIO_PROVIDERS; i++)
    {
        if (providers[i] != nullptr)
        {
            delete providers[i];
            providers[i] = nullptr;
        }
    }
    return Status::Ok;
}

void SoundMixer::setVolume(float volume)
{
    std::lock_guard<std::mutex> lock(mixerMutex);
    if (volume < 0.0f)
        masterVolume = 0.0f;
    else if (volume > 1.0f)
        masterVolume = 1.0f;
    else
        masterVolume = volume;
}

Status SoundMixer::addSoundProvider(SoundProvider* provider)
{
    std::lock_guard<std::mutex> lock(mixerMutex);
    for (size_t i = 0; i < SPEAKER_NB_AUDIO_PROVIDERS; i++)
    {
        if (providers[i] == nullptr)
        {
            providers[i] = provider;
            return Status::Ok;
        }
    }
    LOG_WARNING(TAG, "No available slot to add audio provider");
    return Status::NoMemory;
}

void SoundMixer::__internal_task(void* pvParams)
{
    while (running)
    {
        // empty mix buffer
        memset(mixBuffer, 0, sizeof(mixBuffer));

        { // lock source list during processing
            std::lock_guard<std::mutex> lock(mixerMutex);

            for (int source_idx = 0; source_idx < SPEAKER_NB_AUDIO_PROVIDERS; source_idx++)
            {
                SoundProvider* provider = providers[source_idx];
                if (provider == nullptr)
                    continue; // no provider in this slot, skip

                // ask for samples
                bool hasSamples = provider->provideSamples(sourceBuffer, MIX_BUFFER_SIZE);
                if (!hasSamples) // no samples ? CIAO
                {
                    // delete provider
                    delete provider;
                    providers[source_idx] = nullptr; // mark free
                    continue;
                }

                // mix sourceBuffer into mixBuffer
                for (size_t i = 0; i < MIX_BUFFER_SIZE; i++)
                {
                    int32_t mixedSample = static_cast<int32_t>(mixBuffer[i]) + static_cast<int32_t>(sourceBuffer[i]);
                    // clamp to int16_t range
                    if (mixedSample > INT16_MAX)
                        mixedSample = INT16_MAX;
                    else if (mixedSample < INT16_MIN)
                        mixedSample = INT16_MIN;
                    mixBuffer[i] = static_cast<int16_t>(mixedSample);
                }
            }
        }

        // apply master volume
        for (size_t i = 0; i < MIX_BUFFER_SIZE; i++)
        {
            mixBuffer[i] = static_cast<int16_t>(
                static_cast<float>(mixBuffer[i]) * masterVolume
            );
        }

        // send to speaker
        speaker.writeSamples(mixBuffer, MIX_BUFFER_SIZE); // blocking to sync to audio rate (DMA magic uwu)
    }

    // cleanup on exit
    vTaskDelete(nullptr);
}