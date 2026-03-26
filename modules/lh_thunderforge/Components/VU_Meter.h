#pragma once
#include "../lh_thunderforge.h"
#include "../LookAndFeel/ThunderforgeLookAndFeel.h"

namespace thunderforge
{

class VU_Meter : public juce::Component, public juce::Timer
{
public:
    VU_Meter() { startTimerHz (30); }

    void setLevel (float l) { level = l; }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour (juce::Colours::black);
        g.fillRoundedRectangle (bounds, 2.0f);
        
        auto meterArea = bounds.reduced (2);
        int numSegments = 20;
        float segmentHeight = meterArea.getHeight() / numSegments;
        
        for (int i = 0; i < numSegments; ++i)
        {
            float segBoundsY = meterArea.getY() + (numSegments - 1 - i) * segmentHeight;
            auto segRect = juce::Rectangle<float> (meterArea.getX(), segBoundsY, meterArea.getWidth(), segmentHeight - 1);
            
            float segThreshold = (float)i / (float)numSegments;
            
            if (level > segThreshold)
            {
                if (i < 12) g.setColour (ThunderforgeLookAndFeel::tubeGreen);
                else if (i < 17) g.setColour (ThunderforgeLookAndFeel::amberGold);
                else g.setColour (ThunderforgeLookAndFeel::redline);
                g.fillRect (segRect);
            }
            else
            {
                g.setColour (juce::Colours::white.withAlpha (0.05f));
                g.fillRect (segRect);
            }
        }
    }

    void timerCallback() override { repaint(); }

private:
    float level = 0.0f;
};

} // namespace thunderforge
