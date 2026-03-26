#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class Chorus
{
public:
    Chorus() {}

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine (spec.sampleRate * 0.1); // 100ms max
        for (auto& dl : delayLines)
            dl.prepare (spec);
            
        lfo.prepare (spec);
        lfo.initialise ([](float x) { return std::sin (x); });
    }

    void setParameters (float rate, float depth, float mix, bool bypassed)
    {
        lfo.setFrequency (rate);
        this->depth = depth / 100.0f * 0.005f; // 5ms depth
        this->mix = mix / 100.0f;
        isBypassed = bypassed;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (isBypassed) return;

        auto numSamples = buffer.getNumSamples();
        auto numChannels = std::min (buffer.getNumChannels(), 2);

        for (int i = 0; i < numSamples; ++i)
        {
            float lfoVal = lfo.processSample (0.0f);
            
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float dry = buffer.getSample (ch, i);
                
                // Modulated delay time: base 10ms + LFO depth
                float currentDelayTime = 0.010f + (lfoVal * depth);
                delayLines[ch].setDelay (currentDelayTime * fs);
                
                float delayed = delayLines[ch].popSample (ch);
                delayLines[ch].pushSample (ch, dry);
                
                buffer.setSample (ch, i, dry * (1.0f - mix) + delayed * mix);
            }
        }
    }

private:
    double fs = 44100.0;
    float depth = 0.002f;
    float mix = 0.5f;
    bool isBypassed = true;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLines[2];
    juce::dsp::Oscillator<float> lfo;
};

} // namespace thunderforge
