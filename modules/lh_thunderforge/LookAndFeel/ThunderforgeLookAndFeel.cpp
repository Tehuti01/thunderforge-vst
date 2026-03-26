#include "ThunderforgeLookAndFeel.h"

namespace thunderforge
{

const juce::Colour ThunderforgeLookAndFeel::abyss          { 0xff050507 };
const juce::Colour ThunderforgeLookAndFeel::deepNavy       { 0xff0a0e1a };
const juce::Colour ThunderforgeLookAndFeel::charcoalPanel  { 0xff141820 };
const juce::Colour ThunderforgeLookAndFeel::steelPanel     { 0xff1e232e };
const juce::Colour ThunderforgeLookAndFeel::slate          { 0xff2a303d };
const juce::Colour ThunderforgeLookAndFeel::tolexBlack     { 0xff0d0d0f };
const juce::Colour ThunderforgeLookAndFeel::marshallGold   { 0xffc8a84e };
const juce::Colour ThunderforgeLookAndFeel::brightChrome   { 0xffe8edf5 };
const juce::Colour ThunderforgeLookAndFeel::brushedSteel   { 0xff8892a4 };
const juce::Colour ThunderforgeLookAndFeel::darkSteel      { 0xff3a4050 };
const juce::Colour ThunderforgeLookAndFeel::amberGold      { 0xffffb000 };
const juce::Colour ThunderforgeLookAndFeel::signalCyan      { 0xff06b6d4 };
const juce::Colour ThunderforgeLookAndFeel::tubeGreen      { 0xff00ff78 };
const juce::Colour ThunderforgeLookAndFeel::redline        { 0xffff3344 };
const juce::Colour ThunderforgeLookAndFeel::valveOrange    { 0xffff8800 };
const juce::Colour ThunderforgeLookAndFeel::softWhite      { 0xffdce6f0 };
const juce::Colour ThunderforgeLookAndFeel::steelBlue      { 0xff6488b4 };
const juce::Colour ThunderforgeLookAndFeel::dimSteel       { 0xff3e4a5c };
const juce::Colour ThunderforgeLookAndFeel::lcdGreen       { 0xff00ff78 };
const juce::Colour ThunderforgeLookAndFeel::lcdAmber       { 0xffffb000 };

const juce::Colour ThunderforgeLookAndFeel::aeroDark       { 0xff050507 };
const juce::Colour ThunderforgeLookAndFeel::aeroPanel      { 0xff101216 };
const juce::Colour ThunderforgeLookAndFeel::aeroPanelLight { 0xff1a1c23 };
const juce::Colour ThunderforgeLookAndFeel::aeroBorder     { 0xff22252e };
const juce::Colour ThunderforgeLookAndFeel::aeroCyan       { 0xff00e5ff };
const juce::Colour ThunderforgeLookAndFeel::aeroPurple     { 0xff9d4edd };
const juce::Colour ThunderforgeLookAndFeel::aeroTextDim     { 0xff64748b };

ThunderforgeLookAndFeel::ThunderforgeLookAndFeel()
{
}

void ThunderforgeLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                               juce::Slider& slider)
{
    auto accent = slider.getName() == "Drive" ? aeroPurple : aeroCyan;
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10);
    auto radius = std::min (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = 3.0f;
    auto arcRadius = radius + 4.0f;

    // 1. Shadow / Track Background
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillEllipse (bounds.expanded (2));

    // 2. Data Ring (LED Arc)
    juce::Path trackPath;
    trackPath.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (aeroDark.brighter (0.1f));
    g.strokePath (trackPath, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valuePath;
    valuePath.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
    
    // Glow effect for the data ring
    g.setColour (accent.withAlpha (0.2f));
    g.strokePath (valuePath, juce::PathStrokeType (lineW + 2.0f));
    
    g.setColour (accent);
    g.strokePath (valuePath, juce::PathStrokeType (lineW));

    // 3. Main Knob Body (Machined Metal look)
    auto knobBounds = bounds.reduced (2);
    juce::ColourGradient knobGrad (aeroPanelLight, knobBounds.getCentreX(), knobBounds.getY(),
                                   aeroPanel, knobBounds.getCentreX(), knobBounds.getBottom(), false);
    g.setGradientFill (knobGrad);
    g.fillEllipse (knobBounds);

    // Subtle edge highlight
    g.setColour (juce::Colours::white.withAlpha (0.1f));
    g.drawEllipse (knobBounds, 1.0f);

    // 4. Machined Grip (Circles/Ridges) - Simplified for vector
    for (float r = 0.4f; r < 0.9f; r += 0.15f)
    {
        g.setColour (juce::Colours::black.withAlpha (0.15f));
        g.drawEllipse (knobBounds.withSizeKeepingCentre (knobBounds.getWidth() * r, knobBounds.getHeight() * r), 0.5f);
    }

    // 5. Center Cap
    auto capBounds = knobBounds.reduced (knobBounds.getWidth() * 0.15f);
    juce::ColourGradient capGrad (aeroPanel.brighter (0.1f), capBounds.getCentreX(), capBounds.getCentreY(),
                                  aeroPanel.darker (0.2f), capBounds.getCentreX(), capBounds.getBottom(), false);
    g.setGradientFill (capGrad);
    g.fillEllipse (capBounds);

    // 6. Indicator (Machined pin look)
    juce::Path indicator;
    auto indicatorWidth = 2.0f;
    auto indicatorLen = capBounds.getWidth() * 0.25f;
    indicator.addRoundedRectangle (-indicatorWidth * 0.5f, -capBounds.getHeight() * 0.45f, indicatorWidth, indicatorLen, 1.0f);
    indicator.applyTransform (juce::AffineTransform::rotation (toAngle).translated (bounds.getCentreX(), bounds.getCentreY()));
    
    g.setColour (accent);
    g.fillPath (indicator);
    
    // Indicator Glow
    g.setColour (accent.withAlpha (0.5f));
    g.strokePath (indicator, juce::PathStrokeType (1.0f));
}

void ThunderforgeLookAndFeel::drawHeader (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text)
{
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions().withName ("Inter").withHeight (28.0f).withStyle ("Bold").withKerningFactor (0.2f));
    g.drawText (text.toUpperCase(), area, juce::Justification::left, true);
    
    // Sub-title logic
    area.removeFromTop (30);
    g.setColour (aeroTextDim);
    g.setFont (juce::FontOptions().withName ("Inter").withHeight (10.0f).withKerningFactor (0.3f));
    g.drawText ("ADVANCED DYNAMICS & EQ", area, juce::Justification::left, true);
}

} // namespace thunderforge
