#pragma once
#include "audio/SoundProvider.hpp"
#include "audio/Speaker.hpp"
#include "common/utils.hpp"

class MusicProvider : public SoundProvider
{
public:
    constexpr static const char* TAG = "MusicProvider";

    MusicProvider();
    ~MusicProvider();

    Status loadFromFile(const char* filepath);

    void stop();

    bool provideSamples(Speaker::Sample* buffer, size_t sampleCount) override;

private:
    char* file_content = nullptr;
    size_t content_size = 0;
    size_t read_position = 0;
};