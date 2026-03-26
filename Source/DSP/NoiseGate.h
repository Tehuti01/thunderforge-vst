#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class NoiseGate
{
public:
    NoiseGate() {}

    void prepare (double sampleRate)
    {
        fs = static_cast<float> (sampleRate);
        envelope.reset (sampleRate, 0.05);
    }

    void setParameters (float thresholdDb, float attackMs, float holdMs, float releaseMs, bool bypassed)
    {
        threshold = juce::Decibels::decibelsToGain (thresholdDb);
        attack = attackMs / 1000.0f;
        hold = holdMs / 1000.0f;
        release = releaseMs / 1000.0f;
        isBypassed = bypassed;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (isBypassed) return;

        auto* channelData = buffer.getWritePointer (0); // Mono processing assume
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float input = std::abs (channelData[i]);
            
            // Simple envelope follower with hysteresis
            if (input > threshold)
            {
                currentHoldTimer = hold;
                targetGain = 1.0f;
            }
            else
            {
                if (currentHoldTimer > 0)
                {
                    currentHoldTimer -= 1.0f / fs;
                    targetGain = 1.0f;
                }
                else
                {
                    targetGain = 0.0f;
                }
            }

            // Smoothing the gain transition
            float coeff = (targetGain > currentGain) ? 
                (1.0f - std::exp (-1.0f / (attack * fs))) : 
                (1.0f - std::exp (-1.0f / (release * fs)));
            
            currentGain += coeff * (targetGain - currentGain);

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample (ch, i, buffer.getSample (ch, i) * currentGain);
        }
    }

private:
    float fs = 44100.0f;
    float threshold = 0.01f;
    float attack = 0.001f;
    float hold = 0.05f;
    float release = 0.1f;
    bool isBypassed = false;

    float currentGain = 1.0f;
    float targetGain = 1.0f;
    float currentHoldTimer = 0.0f;

    juce::LinearSmoothedValue<float> envelope { 1.0f };
};

} // namespace thunderforge
