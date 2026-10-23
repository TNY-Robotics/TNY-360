#pragma once
#include "common/utils.hpp"
#include "audio/Microphone.hpp"
#include "driver/i2s_pdm.h"

class MicrophonePDM : public Microphone
{
public:
    MicrophonePDM() = default;
    virtual ~MicrophonePDM() = default;

    Status init() override;

    Status deinit() override;

    size_t readSamples(Sample* buffer, size_t sampleCount) override;

private:
    i2s_chan_handle_t i2s_handle = nullptr;
    int32_t dc_offset = 0;
};