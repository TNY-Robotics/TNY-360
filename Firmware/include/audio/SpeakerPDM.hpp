#pragma once
#include "audio/ISpeaker.hpp"

/**
 * @brief Implementation of the ISpeaker interface using PDM (Pulse Density Modulation).
 * This type of speaker module is used in version 1.0 of the TNY-360 robot to play audio through the PAM8406 amplifier.
 */
class SpeakerPDM : public ISpeaker
{
public:
    SpeakerPDM();

    Status init() override;

    Status deinit() override;

    void writeSamples(const Sample* samples, size_t sampleCount) override;

private:
    i2s_chan_handle_t txChannel = nullptr;
    i2s_chan_handle_t tx_handle = nullptr;
};