#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class StereoDelay
{
public:
    StereoDelay() {}

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine (spec.sampleRate * 2.0); // 2 seconds max
        delayLines[0] = delayLine;
        delayLines[1] = delayLine;
        
        for (auto& dl : delayLines)
            dl.prepare (spec);
            
        lpFilter.setCoefficients (juce::IIRCoefficients::makeLowPass (fs, 12000.0));
    }

    void setParameters (float timeMs, float feedback, float mix, bool bypassed)
    {
        delayTimeSamples = timeMs * fs / 1000.0f;
        this->feedback = feedback / 100.0f;
        this->mix = mix / 100.0f;
        isBypassed = bypassed;

        for (auto& dl : delayLines)
            dl.setDelay (delayTimeSamples);
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (isBypassed) return;

        auto numSamples = buffer.getNumSamples();
        auto numChannels = std::min (buffer.getNumChannels(), 2);

        for (int i = 0; i < numSamples; ++i)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float dry = buffer.getSample (ch, i);
                float delayed = delayLines[ch].popSample (ch);
                
                // Feedback loop with LP filter
                float fb = lpFilter.processSingleSampleRaw (delayed * feedback);
                delayLines[ch].pushSample (ch, dry + fb);
                
                buffer.setSample (ch, i, dry * (1.0f - mix) + delayed * mix);
            }
        }
    }

private:
    double fs = 44100.0;
    float delayTimeSamples = 0.0f;
    float feedback = 0.4f;
    float mix = 0.3f;
    bool isBypassed = false;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLines[2];
    juce::IIRFilter lpFilter;
};

} // namespace thunderforge
