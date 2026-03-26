#pragma once

#include <JuceHeader.h>

namespace thunderforge
{

class ThunderforgeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ThunderforgeLookAndFeel();

    // Color Palette
    static const juce::Colour abyss;
    static const juce::Colour deepNavy;
    static const juce::Colour charcoalPanel;
    static const juce::Colour steelPanel;
    static const juce::Colour slate;
    static const juce::Colour tolexBlack;
    static const juce::Colour marshallGold;
    static const juce::Colour brightChrome;
    static const juce::Colour brushedSteel;
    static const juce::Colour darkSteel;
    static const juce::Colour amberGold;
    static const juce::Colour signalCyan;
    static const juce::Colour tubeGreen;
    static const juce::Colour redline;
    static const juce::Colour valveOrange;
    static const juce::Colour softWhite;
    static const juce::Colour steelBlue;
    static const juce::Colour dimSteel;
    static const juce::Colour lcdGreen;
    static const juce::Colour lcdAmber;

    // AeroTone Palette
    static const juce::Colour aeroDark;
    static const juce::Colour aeroPanel;
    static const juce::Colour aeroPanelLight;
    static const juce::Colour aeroBorder;
    static const juce::Colour aeroCyan;
    static const juce::Colour aeroPurple;
    static const juce::Colour aeroTextDim;

    // Golden Ratio Constants
    static constexpr float PHI = 1.618033988749895f;
    static constexpr float PHI_INV = 0.618033988749895f;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawHeader (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text);
};

} // namespace thunderforge
