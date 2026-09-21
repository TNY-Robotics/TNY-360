#include "audio/AudioManager.hpp"
#include "settings/Settings.hpp"
#include "audio/SpeakerPDM.hpp"
#include "audio/SpeakerI2S.hpp"
#include "common/Log.hpp"

AudioManager::AudioManager()
{
}

Status AudioManager::init()
{
    LOG_SCOPE(TAG, "AudioManager::Init");
    // Find what type of speaker module is in settings
    AudioSpeakerType speakerType = Settings::GetConfig().audio.speakerType;
    if (speakerType == AudioSpeakerType::SPEAKER_PDM)
    {
        speaker = new SpeakerPDM();
    }
    else if (speakerType == AudioSpeakerType::SPEAKER_I2S)
    {
        speaker = new SpeakerI2S();
    }
    else
    {
        LOG_ERROR(TAG, "Unknown speaker type in settings. Disabling audio.");
        speaker = nullptr;
    }

    if (speaker)
    {
        if (Status err = speaker->init(); err != Status::Ok)
        {
            return err;
        }

        if (Status err = mixer.init(speaker); err != Status::Ok)
        {
            return err;
        }
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
    }

    return Status::Ok;
}
