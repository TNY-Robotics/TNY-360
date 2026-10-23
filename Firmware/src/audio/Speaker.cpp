#include "audio/Speaker.hpp"
#include "audio/SpeakerPDM.hpp"
#include "audio/SpeakerI2S.hpp"
#include "settings/Settings.hpp"

Speaker* Speaker::Create()
{
    // Determine which Speaker to use based on settings
    SpeakerType spkr_type = Settings::GetConfig().audio.speaker.type;
    
    if (spkr_type == SpeakerType::SPEAKER_PDM)
    {
        return new SpeakerPDM();
    }
    else if (spkr_type == SpeakerType::SPEAKER_I2S)
    {
        return new SpeakerI2S();
    }
    return nullptr;
}