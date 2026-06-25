#include "audio/AudioManager.hpp"

AudioManager::AudioManager() : mixer(speaker)
{
}

Status AudioManager::init()
{
    if (Status err = speaker.init(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = mixer.init(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}

Status AudioManager::deinit()
{
    if (Status err = mixer.deinit(); err != Status::Ok)
    {
        return err;
    }

    if (Status err = speaker.deinit(); err != Status::Ok)
    {
        return err;
    }

    return Status::Ok;
}
