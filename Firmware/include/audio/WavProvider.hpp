#pragma once
#include "audio/SoundProvider.hpp"
#include "common/utils.hpp"

class WavProvider : public SoundProvider
{
public:
    constexpr static const char* TAG = "WavProvider";

    WavProvider();

    Error loadFromFile(const char* filepath);

    bool provideSamples(Speaker::Sample* buffer, size_t sampleCount) override;

private:
    char* file_content;
    size_t content_size;
    size_t read_position;
};
