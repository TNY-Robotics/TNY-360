#include "audio/Microphone.hpp"
#include "audio/MicrophonePDM.hpp"
#include "audio/MicrophoneI2S.hpp"
#include "settings/Settings.hpp"

Microphone* Microphone::Create()
{
    MicrophoneType mic_type = Settings::GetConfig().audio.microphone.type;
    
    if (mic_type == MicrophoneType::MICROPHONE_PDM)
    {
        return new MicrophonePDM();
    }
    else if (mic_type == MicrophoneType::MICROPHONE_I2S)
    {
        return new MicrophoneI2S();
    }
    
    return nullptr;
}