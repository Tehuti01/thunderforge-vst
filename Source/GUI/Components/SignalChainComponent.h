#pragma once

#include <JuceHeader.h>
#include "../LookAndFeel/ThunderforgeLookAndFeel.h"

namespace thunderforge
{

class SignalChainComponent : public juce::Component
{
public:
    SignalChainComponent() {}

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour (ThunderforgeLookAndFeel::deepNavy);
        g.fillRect (bounds);
        
        g.setColour (ThunderforgeLookAndFeel::softWhite.withAlpha (0.5f));
        g.setFont (juce::Font ("JetBrains Mono", 14.0f, juce::Font::bold));
        g.drawText ("SIGNAL CHAIN", bounds.removeFromTop (30).reduced (20, 0), juce::Justification::left, true);
        
        // Draw dashed lines between pedal blocks (conceptual)
        g.setColour (ThunderforgeLookAndFeel::softWhite.withAlpha (0.1f));
        float dashes[] = { 4.0f, 4.0f };
        g.drawDashedLine (juce::Line<float> (50, bounds.getCentreY(), getWidth() - 50, bounds.getCentreY()), dashes, 2, 1.0f);
    }

private:
};

} // namespace thunderforge
