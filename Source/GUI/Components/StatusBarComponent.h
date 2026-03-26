#pragma once

#include <JuceHeader.h>
#include "../LookAndFeel/ThunderforgeLookAndFeel.h"

namespace thunderforge
{

class StatusBarComponent : public juce::Component
{
public:
    StatusBarComponent() {}

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour (ThunderforgeLookAndFeel::abyss);
        g.fillRect (bounds);
        
        g.setColour (ThunderforgeLookAndFeel::tubeGreen);
        g.setFont (juce::Font ("JetBrains Mono", 12.0f, juce::Font::plain));
        
        juce::String statusText = "CPU: 12% | 48kHz / 128 | Latency: 2.9ms | Model: Plexi '68 Bright";
        g.drawText (statusText.toUpperCase(), bounds.reduced (20, 0), juce::Justification::centredLeft, true);
    }

private:
};

} // namespace thunderforge
