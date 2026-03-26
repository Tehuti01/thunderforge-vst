#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class Compressor
{
public:
    Compressor() {}

    void prepare (double sampleRate)
    {
        fs = static_cast<float> (sampleRate);
    }

    void setParameters (float thresholdDb, float ratio, float attackMs, float releaseMs, float makeupDb, bool bypassed)
    {
        threshold = thresholdDb;
        this->ratio = ratio;
        attack = 1.0f - std::exp (-1.0f / (attackMs / 1000.0f * fs));
        release = 1.0f - std::exp (-1.0f / (releaseMs / 1000.0f * fs));
        makeup = juce::Decibels::decibelsToGain (makeupDb);
        isBypassed = bypassed;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (isBypassed) return;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float rms = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                rms += buffer.getSample (ch, i) * buffer.getSample (ch, i);
            
            rms = std::sqrt (rms / buffer.getNumChannels());
            float levelDb = juce::Decibels::gainToDecibels (rms, -120.0f);
            
            float targetReductionDb = 0.0f;
            if (levelDb > threshold)
                targetReductionDb = (levelDb - threshold) * (1.0f - 1.0f / ratio);
            
            // Smoothing the reduction
            if (targetReductionDb > currentReductionDb)
                currentReductionDb += attack * (targetReductionDb - currentReductionDb);
            else
                currentReductionDb += release * (targetReductionDb - currentReductionDb);
            
            float gainMult = juce::Decibels::decibelsToGain (-currentReductionDb) * makeup;
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample (ch, i, buffer.getSample (ch, i) * gainMult);
        }
    }

private:
    float fs = 44100.0f;
    float threshold = -20.0f;
    float ratio = 4.0f;
    float attack = 0.01f;
    float release = 0.1f;
    float makeup = 1.0f;
    bool isBypassed = true;

    float currentReductionDb = 0.0f;
};

} // namespace thunderforge
