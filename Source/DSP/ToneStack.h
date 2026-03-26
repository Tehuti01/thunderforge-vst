#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class ToneStack
{
public:
    ToneStack() {}

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        updateFilters();
    }

    void setParameters (float bass, float mid, float treble, float presence)
    {
        this->bassGain = (bass - 5.0f) * 2.4f;      // +/- 12dB
        this->midGain = (mid - 5.0f) * 2.4f;        // +/- 12dB
        this->trebleGain = (treble - 5.0f) * 2.4f;  // +/- 12dB
        this->presenceGain = (presence - 5.0f) * 1.2f; // +/- 6dB
        
        updateFilters();
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = data[i];
                x = bassFilter.processSingleSampleRaw (x);
                x = midFilter.processSingleSampleRaw (x);
                x = trebleFilter.processSingleSampleRaw (x);
                x = presenceFilter.processSingleSampleRaw (x);
                data[i] = x;
            }
        }
    }

private:
    void updateFilters()
    {
        bassFilter.setCoefficients (juce::IIRCoefficients::makeLowShelf (fs, 100.0, 0.7, juce::Decibels::decibelsToGain (bassGain)));
        midFilter.setCoefficients (juce::IIRCoefficients::makePeakFilter (fs, 800.0, 0.7, juce::Decibels::decibelsToGain (midGain)));
        trebleFilter.setCoefficients (juce::IIRCoefficients::makeHighShelf (fs, 3200.0, 0.7, juce::Decibels::decibelsToGain (trebleGain)));
        presenceFilter.setCoefficients (juce::IIRCoefficients::makeHighShelf (fs, 5000.0, 0.7, juce::Decibels::decibelsToGain (presenceGain)));
    }

    double fs = 44100.0;
    float bassGain = 0.0f;
    float midGain = 0.0f;
    float trebleGain = 0.0f;
    float presenceGain = 0.0f;

    juce::IIRFilter bassFilter;
    juce::IIRFilter midFilter;
    juce::IIRFilter trebleFilter;
    juce::IIRFilter presenceFilter;
};

} // namespace thunderforge
