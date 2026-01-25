#include "audio/WavProvider.hpp"
#include "common/LittleFS.hpp"
#include "common/Log.hpp"
#include <memory.h>

WavProvider::WavProvider()
{
}

Error WavProvider::loadFromFile(const char* filepath)
{
    if (Error err = LittleFS::Init(); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to initialize LittleFS: %d", ErrorToString(err));
        return err;
    }

    if (Error err = LittleFS::LoadFileContent(filepath, &file_content, &content_size); err != Error::None)
    {
        Log::Add(Log::Level::Error, TAG, "Failed to load WAV file '%s': %d", filepath, ErrorToString(err));
        return err;
    }
    read_position = 0;
    
    return Error::None;
}

bool WavProvider::provideSamples(Speaker::Sample* buffer, size_t sampleCount)
{
    size_t samplesProvided = 0;
    while (samplesProvided < sampleCount && read_position + sizeof(Speaker::Sample) <= content_size)
    {
        // Copy sample
        memcpy(&buffer[samplesProvided], &file_content[read_position], sizeof(Speaker::Sample));
        read_position += sizeof(Speaker::Sample);
        samplesProvided++;
    }

    return samplesProvided > 0;
}