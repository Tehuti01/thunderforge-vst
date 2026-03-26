#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class WaveShaper
{
public:
    WaveShaper() {}

    void prepare (double sampleRate)
    {
        fs = static_cast<float> (sampleRate);
    }

    void setParameters (float gainValue)
    {
        gain = gainValue;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = data[i] * gain;
                
                // Asymmetric tube-like saturation
                if (x >= 0.0f)
                    x = std::tanh (x * 1.2f);
                else
                    x = std::tanh (x * 0.8f) * 0.95f;
                
                data[i] = x;
            }
        }
    }

private:
    float fs = 44100.0f;
    float gain = 1.0f;
};

} // namespace thunderforge
