#include "audio/AudioManager.hpp"
#include "settings/Settings.hpp"
#include "audio/Speaker.hpp"
#include "common/Log.hpp"

AudioManager::AudioManager()
{
}

Status AudioManager::init()
{
    LOG_SCOPE(TAG, "AudioManager::Init");

    speaker = Speaker::Create();
    if (speaker)
    {
        if (Status err = speaker->init(); err != Status::Ok)
        {
            return err;
        }
    }
    else
    {
        LOG_INFO(TAG, "Speaker creation returned null. Disabling audio output.");
    }
    
    if (Status err = mixer.init(speaker); err != Status::Ok)
    {
        return err;
    }

    microphone = Microphone::Create();
    if (microphone)
    {
        if (Status err = microphone->init(); err != Status::Ok)
        {
            return err;
        }
    }
    else
    {
        LOG_INFO(TAG, "Microphone creation returned null. Disabling audio input.");
    }

    return Status::Ok;
}

Status AudioManager::deinit()
{
    if (speaker)
    {
        if (Status err = mixer.deinit(); err != Status::Ok)
        {
            return err;
        }

        if (Status err = speaker->deinit(); err != Status::Ok)
        {
            return err;
        }

        delete speaker;
        speaker = nullptr;
    }

    if (microphone)
    {
        if (Status err = microphone->deinit(); err != Status::Ok)
        {
            return err;
        }

        delete microphone;
        microphone = nullptr;
    }

    return Status::Ok;
}
