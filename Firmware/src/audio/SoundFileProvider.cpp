#include "audio/SoundFileProvider.hpp"
#include "common/LittleFS.hpp"
#include "common/Log.hpp"
#include <cstring>
#include <cstdlib>
#include <algorithm>

SoundFileProvider::SoundFileProvider(const char* filepath)
    : file_content(nullptr), content_size(0), read_position(0), m_isFirstBuffer(true), m_stopRequested(false)
{
    if (filepath != nullptr)
    {
        loadFromFile(filepath);
    }
}

SoundFileProvider::~SoundFileProvider()
{
    cleanup();
}

void SoundFileProvider::cleanup()
{
    if (file_content != nullptr)
    {
        free(file_content);
        file_content = nullptr;
    }
    content_size = 0;
    read_position = 0;
    m_isFirstBuffer = true;
    m_stopRequested = false;
}

Status SoundFileProvider::loadFromFile(const char* filepath)
{
    cleanup();

    if (Status err = LittleFS::Init(); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to initialize LittleFS: %d", static_cast<int>(err));
        return err;
    }

    if (Status err = LittleFS::GetFileSize(filepath, content_size); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to get size of file '%s': %d", filepath, static_cast<int>(err));
        return err;
    }

    if (content_size < 44)
    {
        LOG_ERROR(TAG, "File '%s' too small to be a valid WAV audio file (%u bytes)", filepath, content_size);
        cleanup();
        return Status::InvalidParameters;
    }

    file_content = static_cast<char*>(malloc(content_size));
    if (file_content == nullptr)
    {
        LOG_ERROR(TAG, "Memory allocation failed for '%s' (%u bytes)", filepath, content_size);
        cleanup();
        return Status::NoMemory;
    }

    size_t bytes_read = 0;
    if (Status err = LittleFS::ReadFile(filepath, file_content, content_size, bytes_read); err != Status::Ok)
    {
        LOG_ERROR(TAG, "Failed to read WAV file '%s': %d", filepath, static_cast<int>(err));
        cleanup();
        return err;
    }

    read_position = 0;
    for (size_t i = 12; i <= content_size - 8; ++i)
    {
        if (file_content[i] == 'd' && file_content[i+1] == 'a' && 
            file_content[i+2] == 't' && file_content[i+3] == 'a')
        {
            read_position = i + 8;
            break;
        }
    }

    if (read_position == 0 || read_position >= content_size)
    {
        LOG_WARNING(TAG, "No 'data' chunk found in '%s'. Falling back to offset 44.", filepath);
        read_position = 44;
    }

    if (read_position % sizeof(Speaker::Sample) != 0)
    {
        read_position++;
    }

    m_isFirstBuffer = true;
    m_stopRequested = false;

    LOG_INFO(TAG, "Loaded WAV '%s' (Size: %u, Data offset: %u)", filepath, content_size, read_position);
    return Status::Ok;
}

void SoundFileProvider::stop()
{
    m_stopRequested = true;
}

bool SoundFileProvider::provideSamples(Speaker::Sample* buffer, size_t sampleCount)
{
    if (file_content == nullptr || sampleCount == 0)
    {
        return false;
    }

    if (m_stopRequested || read_position >= content_size)
    {
        std::fill(buffer, buffer + sampleCount, static_cast<Speaker::Sample>(0));
        return false;
    }

    size_t bytesLeft = content_size - read_position;
    size_t bytesToRead = sampleCount * sizeof(Speaker::Sample);
    bool reachedEnd = false;

    if (bytesToRead >= bytesLeft)
    {
        bytesToRead = bytesLeft;
        reachedEnd = true;
    }

    bytesToRead -= (bytesToRead % sizeof(Speaker::Sample));
    size_t samplesRead = bytesToRead / sizeof(Speaker::Sample);

    std::memcpy(buffer, file_content + read_position, bytesToRead);
    read_position += bytesToRead;

    if (samplesRead < sampleCount)
    {
        std::fill(buffer + samplesRead, buffer + sampleCount, static_cast<Speaker::Sample>(0));
    }

    if (m_isFirstBuffer && samplesRead > 0)
    {
        constexpr size_t kFadeInSamples = 64;
        size_t count = std::min(samplesRead, kFadeInSamples);
        for (size_t i = 0; i < count; ++i)
        {
            float factor = static_cast<float>(i) / static_cast<float>(count);
            buffer[i] = static_cast<Speaker::Sample>(buffer[i] * factor);
        }
        m_isFirstBuffer = false;
    }

    if (reachedEnd && samplesRead > 0)
    {
        constexpr size_t kFadeOutSamples = 64;
        size_t count = std::min(samplesRead, kFadeOutSamples);
        size_t startIndex = samplesRead - count;
        for (size_t i = 0; i < count; ++i)
        {
            float factor = 1.0f - (static_cast<float>(i) / static_cast<float>(count));
            buffer[startIndex + i] = static_cast<Speaker::Sample>(buffer[startIndex + i] * factor);
        }
        return false;
    }

    return true;
}