#pragma once

#include "common/utils.hpp"
#include "audio/Microphone.hpp"
#include <driver/i2s_std.h>
#include <cstdint>

class MicrophoneI2S : public Microphone
{
public:
    MicrophoneI2S() = default;
    virtual ~MicrophoneI2S() override { deinit(); }

    Status init() override;
    Status deinit() override;
    size_t readSamples(Sample* buffer, size_t sampleCount) override;

private:
    i2s_chan_handle_t i2s_handle = nullptr;
    int32_t dc_offset = 0;
};