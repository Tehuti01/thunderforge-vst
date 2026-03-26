#pragma once
#include "../lh_thunderforge.h"
#include "../LookAndFeel/ThunderforgeLookAndFeel.h"

namespace thunderforge
{

class LEDComponent : public juce::Component
{
public:
    LEDComponent (juce::Colour colour = juce::Colours::red) : ledColour (colour) {}

    void setState (bool on) { isOn = on; repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1.0f);
        
        // Casing
        g.setColour (ThunderforgeLookAndFeel::charcoalPanel);
        g.fillEllipse (bounds);
        
        // LED Surface
        if (isOn)
        {
            juce::ColourGradient grad (ledColour.brighter (0.8f), bounds.getCentreX(), bounds.getCentreY(),
                                       ledColour, bounds.getX(), bounds.getY(), true);
            g.setGradientFill (grad);
            g.fillEllipse (bounds.reduced (1.0f));
            
            // Halo glow
            g.setColour (ledColour.withAlpha (0.2f));
            g.drawEllipse (bounds.expanded (2.0f), 1.0f);
        }
        else
        {
            g.setColour (ledColour.withAlpha (0.2f));
            g.fillEllipse (bounds.reduced (1.0f));
        }
    }

private:
    juce::Colour ledColour;
    bool isOn = true;
};

} // namespace thunderforge
