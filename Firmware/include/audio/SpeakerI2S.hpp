#pragma once
#include "audio/ISpeaker.hpp"

/**
 * @brief Implementation of the ISpeaker interface using only I2S (Inter-IC Sound) protocol.
 * This type of speaker module is used in version 2.0 of the TNY-360 robot to play audio through the NS4168 amplifier.
 */
class SpeakerI2S : public ISpeaker
{
public:
    SpeakerI2S();

    Status init() override;

    Status deinit() override;

    void writeSamples(const Sample* samples, size_t sampleCount) override;

private:
    i2s_chan_handle_t txChannel = nullptr;
};