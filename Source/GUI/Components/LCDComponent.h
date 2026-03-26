#pragma once

#include <JuceHeader.h>
#include "../LookAndFeel/ThunderforgeLookAndFeel.h"

namespace thunderforge
{

class LCDComponent : public juce::Component, public juce::Timer
{
public:
    LCDComponent()
    {
        startTimerHz (30);
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // 1. Background (Dark Liquid LCD)
        g.setColour (ThunderforgeLookAndFeel::aeroDark.darker (0.2f));
        g.fillRoundedRectangle (bounds, 12.0f);

        // 2. Grid Lines
        g.setColour (juce::Colours::white.withAlpha (0.02f));
        float gridSize = 30.0f;
        for (float x = gridSize; x < bounds.getWidth(); x += gridSize)
            g.drawVerticalLine ((int)x, 0.0f, bounds.getHeight());
        for (float y = gridSize; y < bounds.getHeight(); y += gridSize)
            g.drawHorizontalLine ((int)y, 0.0f, bounds.getWidth());

        // 3. Spectrum Rendering (Smooth Quadratic Curve)
        drawSpectrum (g, bounds);

        // 1. Frequency Label
        g.setColour (ThunderforgeLookAndFeel::aeroTextDim);
        g.setFont (juce::FontOptions().withName ("JetBrains Mono").withHeight (12.0f));
        g.drawText (juce::String::formatted ("PEAK: %.0f Hz", currentHz), bounds.removeFromTop (20).reduced (10, 5), juce::Justification::centredRight);
        
        // 2. Main Module Title
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.setFont (juce::FontOptions().withName ("Inter").withHeight (24.0f).withStyle ("Bold"));
        g.drawText ("GLASS LCD VISUALIZER", bounds.removeFromTop (30).reduced (20, 0), juce::Justification::centredLeft);

        // 5. Glass Glare Overlay
        juce::ColourGradient glare (juce::Colours::white.withAlpha (0.05f), 0, 0,
                                   juce::Colours::transparentWhite, bounds.getWidth() * 0.5f, bounds.getHeight() * 0.5f, false);
        g.setGradientFill (glare);
        g.fillRoundedRectangle (bounds, 12.0f);
        
        // 6. Bezel Shadow
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds, 12.0f, 2.0f);
    }

    void timerCallback() override
    {
        repaint();
    }

    void pushNextFFTData (const std::vector<float>& newData, float peakHz)
    {
        fftData = newData;
        currentHz = peakHz;
    }

private:
    void drawSpectrum (juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        if (fftData.empty()) return;

        juce::Path p;
        auto w = bounds.getWidth();
        auto h = bounds.getHeight();
        auto numBins = (float)fftData.size();

        p.startNewSubPath (0, h);

        auto minFreq = 20.0f;
        auto maxFreq = 20000.0f;
        auto sampleRate = 44100.0f; // Simplified for mapping

        for (int i = 0; i < fftData.size(); ++i)
        {
            float freq = (float)i * sampleRate / (float)(fftData.size() * 2);
            if (freq < minFreq) freq = minFreq;
            
            // Logarithmic X mapping
            auto x = juce::jmap (std::log10 (freq), std::log10 (minFreq), std::log10 (maxFreq), 0.0f, w);
            
            auto val = std::pow (fftData[i], 1.5f); // Increased punch
            auto y = h - (val * h * 0.85f);
            
            if (i == 0) p.lineTo (0, y);
            else
            {
                p.lineTo (x, y);
            }
        }

        p.lineTo (w, h);
        p.closeSubPath();

        // Layered Fill
        juce::ColourGradient fill (ThunderforgeLookAndFeel::aeroCyan.withAlpha (0.6f), 0, 0,
                                   ThunderforgeLookAndFeel::aeroCyan.withAlpha (0.0f), 0, h, false);
        g.setGradientFill (fill);
        g.fillPath (p);

        // Outline
        g.setColour (ThunderforgeLookAndFeel::aeroCyan.withAlpha (0.8f));
        g.strokePath (p, juce::PathStrokeType (2.0f));
    }

    std::vector<float> fftData;
    float currentHz = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LCDComponent)
};

} // namespace thunderforge
