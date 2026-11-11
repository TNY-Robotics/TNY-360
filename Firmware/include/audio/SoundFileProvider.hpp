#pragma once
#include "audio/SoundProvider.hpp"
#include "audio/Speaker.hpp"
#include "common/utils.hpp"

class SoundFileProvider : public SoundProvider
{
public:
    constexpr static const char* TAG = "SoundFileProvider";

    SoundFileProvider(const char* filepath);
    SoundFileProvider(const SoundFileProvider&) = delete;
    SoundFileProvider& operator=(const SoundFileProvider&) = delete;
    ~SoundFileProvider() override;

    Status loadFromFile(const char* filepath);
    void stop();

    bool provideSamples(Speaker::Sample* buffer, size_t sampleCount) override;

private:
    void cleanup();

    char*  file_content = nullptr;
    size_t content_size = 0;
    size_t read_position = 0;

    bool   m_isFirstBuffer = true;
    bool   m_stopRequested = false;
};