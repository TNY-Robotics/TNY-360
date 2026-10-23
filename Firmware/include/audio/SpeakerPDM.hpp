#pragma once
#include "audio/Speaker.hpp"
#include "driver/i2s_std.h"

/**
 * @brief Implementation of the Speaker class using PDM (Pulse Density Modulation).
 * This type of speaker module is used in version 1.0 of the TNY-360 robot to play audio through the PAM8406 amplifier.
 */
class SpeakerPDM : public Speaker
{
public:
    SpeakerPDM() = default;
    virtual ~SpeakerPDM() = default;

    Status init() override;

    Status deinit() override;

    void writeSamples(const Sample* samples, size_t sampleCount) override;

private:
    i2s_chan_handle_t txChannel = nullptr;
    i2s_chan_handle_t tx_handle = nullptr;
};