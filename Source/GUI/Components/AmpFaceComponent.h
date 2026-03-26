#pragma once

#include <JuceHeader.h>
#include "../LookAndFeel/ThunderforgeLookAndFeel.h"

namespace thunderforge
{

class AmpFaceComponent : public juce::Component
{
public:
    AmpFaceComponent() {}

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Layer 0: Tolex texture background
        g.setColour (ThunderforgeLookAndFeel::tolexBlack);
        g.fillRoundedRectangle (bounds, 10.0f);
        
        // Layer 1: Chrome chassis plate
        auto chassisArea = bounds.reduced (20, 40);
        juce::ColourGradient chassisGrad (ThunderforgeLookAndFeel::brushedSteel, chassisArea.getX(), chassisArea.getY(),
                                          ThunderforgeLookAndFeel::charcoalPanel, chassisArea.getRight(), chassisArea.getBottom(), false);
        g.setGradientFill (chassisGrad);
        g.fillRoundedRectangle (chassisArea, 5.0f);
        
        // Layer 2: Control panel (dark faceplate)
        auto panelArea = chassisArea.reduced (10);
        g.setColour (ThunderforgeLookAndFeel::tolexBlack.brighter (0.05f));
        g.fillRoundedRectangle (panelArea, 3.0f);
        
        // Trim / Piping
        g.setColour (ThunderforgeLookAndFeel::marshallGold);
        g.drawRoundedRectangle (panelArea, 3.0f, 1.5f);
        
        // Logo
        g.setColour (ThunderforgeLookAndFeel::marshallGold);
        g.setFont (juce::Font ("Cabinet Grotesk", 32.0f, juce::Font::bold));
        g.drawText ("L H   T H U N D E R F O R G E", panelArea.removeFromBottom (60), juce::Justification::centred, true);
    }

private:
};

} // namespace thunderforge
