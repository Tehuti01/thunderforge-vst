#pragma once
#include "../lh_thunderforge.h"
#include "../LookAndFeel/ThunderforgeLookAndFeel.h"

namespace thunderforge
{

class KnobComponent : public juce::Slider
{
public:
    KnobComponent (const juce::String& labelText = "") : label (labelText)
    {
        setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    }

    void setLevel (float newLevel) 
    { 
        if (std::abs(level - newLevel) > 0.01f)
        {
            level = newLevel; 
            repaint(); 
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto centre = bounds.getCentre();
        auto radius = std::min (bounds.getWidth(), bounds.getHeight()) / 2.2f;

        // Draw Marks (Ticks)
        g.setColour (ThunderforgeLookAndFeel::softWhite.withAlpha (0.3f));
        for (int i = 0; i < 11; ++i)
        {
            auto angle = juce::jmap ((float)i, 0.0f, 10.0f, -juce::MathConstants<float>::pi * 0.75f, juce::MathConstants<float>::pi * 0.75f);
            auto p1 = centre.getPointOnCircumference (radius + 2.0f, angle);
            auto p2 = centre.getPointOnCircumference (radius + 5.0f, angle);
            g.drawLine (p1.x, p1.y, p2.x, p2.y, 1.5f);
        }

        // Draw Dynamic Glow
        if (level > 0.01f)
        {
            auto bounds = getLocalBounds().toFloat().reduced (8.0f);
            auto centre = bounds.getCentre();
            auto radius = std::min (bounds.getWidth(), bounds.getHeight()) / 2.0f;
            
            juce::Path glowPath;
            glowPath.addCentredArc (centre.x, centre.y, radius + 2.0f, radius + 2.0f, 0.0f, 0.0f, juce::MathConstants<float>::twoPi, true);
            
            g.setColour (ThunderforgeLookAndFeel::aeroCyan.withAlpha (level * 0.4f));
            g.strokePath (glowPath, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        juce::Slider::paint (g);
        
        // Draw label below
        if (label.isNotEmpty())
        {
            auto bounds = getLocalBounds();
            g.setColour (ThunderforgeLookAndFeel::softWhite.withAlpha (0.7f));
            g.setFont (juce::FontOptions().withName ("JetBrains Mono").withHeight (12.0f));
            g.drawText (label, bounds.removeFromBottom (15), juce::Justification::centred, true);
        }
    }

private:
    juce::String label;
    float level = 0.0f;
};

} // namespace thunderforge
