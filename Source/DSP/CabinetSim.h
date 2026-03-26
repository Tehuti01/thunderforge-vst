#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class CabinetSim
{
public:
    CabinetSim() {}

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        convolution.prepare (spec);
    }

    void loadIR (const void* sourceData, size_t sourceDataSize, juce::dsp::Convolution::Stereo isStereo, juce::dsp::Convolution::Trim trimIR)
    {
        convolution.loadImpulseResponse (sourceData, sourceDataSize, isStereo, trimIR, 0);
    }

    void process (juce::dsp::ProcessContextReplacing<float>& context)
    {
        if (isBypassed) return;
        convolution.process (context);
    }

    void setBypass (bool bypass) { isBypassed = bypass; }

private:
    juce::dsp::Convolution convolution;
    bool isBypassed = false;
};

} // namespace thunderforge
