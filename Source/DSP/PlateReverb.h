#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class PlateReverb
{
public:
    PlateReverb() {}

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        reverb.setSampleRate (spec.sampleRate);
    }

    void setParameters (float size, float damping, float mix, bool bypassed)
    {
        juce::Reverb::Parameters params;
        params.roomSize = size / 100.0f;
        params.damping = damping / 100.0f;
        params.wetLevel = mix / 100.0f;
        params.dryLevel = 1.0f - (mix / 100.0f);
        
        reverb.setParameters (params);
        isBypassed = bypassed;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (isBypassed) return;

        if (buffer.getNumChannels() == 1)
            reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
        else
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
    }

private:
    juce::Reverb reverb;
    bool isBypassed = false;
};

} // namespace thunderforge
