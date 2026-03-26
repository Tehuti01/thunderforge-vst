#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class TubeScreamer
{
public:
    TubeScreamer() {}

    void prepare (double sampleRate)
    {
        fs = static_cast<float> (sampleRate);
        
        // Input highpass at 720Hz
        hpFilter.setCoefficients (juce::IIRCoefficients::makeHighPass (sampleRate, 720.0));
        
        // Tone control lowpass (sweepable 1kHz to 5kHz)
        lpFilter.setCoefficients (juce::IIRCoefficients::makeLowPass (sampleRate, 1000.0));
    }

    void setParameters (float drive, float tone, float level, bool bypassed)
    {
        driveAmount = 1.0f + (drive / 100.0f * 5.0f);
        toneCutoff = 1000.0f + (tone / 100.0f * 4000.0f);
        outputLevel = level / 100.0f;
        isBypassed = bypassed;

        lpFilter.setCoefficients (juce::IIRCoefficients::makeLowPass (fs, toneCutoff));
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (isBypassed) return;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                // 1. Highpass 720Hz
                float x = hpFilter.processSingleSampleRaw (data[i]);
                
                // 2. Gain staging
                x *= driveAmount;
                
                // 3. Asymmetric soft-clipping (tanh)
                if (x > 0.0f)
                    x = std::tanh (x * 1.1f);
                else
                    x = std::tanh (x);
                
                // 4. Tone control (Lowpass)
                x = lpFilter.processSingleSampleRaw (x);
                
                // 5. Output level
                data[i] = x * outputLevel;
            }
        }
    }

private:
    float fs = 44100.0f;
    float driveAmount = 1.0f;
    float toneCutoff = 1000.0f;
    float outputLevel = 0.5f;
    bool isBypassed = false;

    juce::IIRFilter hpFilter;
    juce::IIRFilter lpFilter;
};

} // namespace thunderforge
