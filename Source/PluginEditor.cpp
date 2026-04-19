#include <JuceHeader.h>
#include <lh_thunderforge/lh_thunderforge.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

ThunderforgeAudioProcessorEditor::ThunderforgeAudioProcessorEditor (ThunderforgeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (lcd);
    addAndMakeVisible (gateKnob);
    addAndMakeVisible (compKnob);
    addAndMakeVisible (gainKnob);
    addAndMakeVisible (bassKnob);
    addAndMakeVisible (midKnob);
    addAndMakeVisible (trebleKnob);
    addAndMakeVisible (delayKnob);
    addAndMakeVisible (reverbKnob);
    addAndMakeVisible (masterKnob);
    addAndMakeVisible (widthKnob); // Added widthKnob

    addAndMakeVisible (testNoteButton);
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);
    addAndMakeVisible (presetLabel);
    addAndMakeVisible (inputMeter);
    addAndMakeVisible (outputMeter);

    presetLabel.setJustificationType (juce::Justification::centred);
    presetLabel.setFont (juce::FontOptions().withName ("JetBrains Mono").withHeight (14.0f));
    presetLabel.setColour (juce::Label::textColourId, thunderforge::ThunderforgeLookAndFeel::aeroCyan);

    prevButton.onClick = [this] {
        int numPresets = audioProcessor.getNumPresets();
        if (numPresets > 0)
        {
            int nextIdx = (audioProcessor.getCurrentPresetIndex() + numPresets - 1) % numPresets;
            audioProcessor.loadPreset (nextIdx);
        }
    };

    nextButton.onClick = [this] {
        int numPresets = audioProcessor.getNumPresets();
        if (numPresets > 0)
        {
            int nextIdx = (audioProcessor.getCurrentPresetIndex() + 1) % numPresets;
            audioProcessor.loadPreset (nextIdx);
        }
    };

    int numButtons = std::min(5, audioProcessor.getNumPresets());
    for (int i = 0; i < numButtons; ++i)
    {
        juce::String name = audioProcessor.getPresetName(i);
        if (name == "HIGHWAY TO HELL") name = "HIGHWAY";
        if (name == "THUNDERSTRUCK") name = "THUNDER";
        if (name == "YOU SHOOK ME") name = "SHOOK ME";

        acdcButtons[i].setButtonText (name);
        addAndMakeVisible (acdcButtons[i]);
        acdcButtons[i].onClick = [this, i] { audioProcessor.loadPreset (i); };
    }

    testNoteButton.onStateChange = [this] { 
        audioProcessor.triggerTestNote (testNoteButton.isMouseButtonDown()); 
    };

    gateAttachment   = std::make_unique<Attachment> (audioProcessor.apvts, "gate_threshold", gateKnob);
    compAttachment   = std::make_unique<Attachment> (audioProcessor.apvts, "comp_threshold", compKnob);
    gainAttachment   = std::make_unique<Attachment> (audioProcessor.apvts, "ts_drive", gainKnob);
    bassAttachment   = std::make_unique<Attachment> (audioProcessor.apvts, "eq_bass", bassKnob);
    midAttachment    = std::make_unique<Attachment> (audioProcessor.apvts, "eq_mid", midKnob);
    trebleAttachment = std::make_unique<Attachment> (audioProcessor.apvts, "eq_treble", trebleKnob);
    delayAttachment  = std::make_unique<Attachment> (audioProcessor.apvts, "delay_mix", delayKnob);
    reverbAttachment = std::make_unique<Attachment> (audioProcessor.getAPVTS(), "reverb_mix", reverbKnob);
    masterAttachment = std::make_unique<Attachment> (audioProcessor.getAPVTS(), "output_gain", masterKnob);
    widthAttachment  = std::make_unique<Attachment> (audioProcessor.getAPVTS(), "stereo_width", widthKnob);

    // Set slider names for LookAndFeel logic
    gainKnob.setName ("Drive");

    addAndMakeVisible (loadNAMButton);
    addAndMakeVisible (loadIRButton);

    loadNAMButton.onClick = [this] {
        chooser = std::make_unique<juce::FileChooser> ("Select NAM Model...", juce::File::getSpecialLocation (juce::File::userHomeDirectory), "*.nam");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        chooser->launchAsync (flags, [this] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) audioProcessor.loadNAMModel (file);
        });
    };

    loadIRButton.onClick = [this] {
        chooser = std::make_unique<juce::FileChooser> ("Select Cabinet IR...", juce::File::getSpecialLocation (juce::File::userHomeDirectory), "*.wav");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        chooser->launchAsync (flags, [this] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) audioProcessor.loadCabinetIR (file);
        });
    };

    setSize (1000, 650);
    startTimerHz (60);
}

ThunderforgeAudioProcessorEditor::~ThunderforgeAudioProcessorEditor() 
{
    setLookAndFeel (nullptr);
}

void ThunderforgeAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.fillAll (thunderforge::ThunderforgeLookAndFeel::aeroDark);
    
    // --- 300x TUBE GLOW ---
    auto driveVal = (float)*audioProcessor.getAPVTS().getRawParameterValue ("ts_drive") / 100.0f;
    auto widthVal = (float)*audioProcessor.getAPVTS().getRawParameterValue ("stereo_width") / 200.0f;
    auto peak     = audioProcessor.getPeakLevel();
    auto glowAlpha = (driveVal * 0.3f + peak * 0.2f + widthVal * 0.1f) * (0.8f + 0.2f * std::sin (juce::Time::getMillisecondCounterHiRes() * 0.005));
    
    auto area = getLocalBounds().toFloat().reduced (40);
    auto preampArea = area.removeFromLeft (140).reduced (10);
    
    juce::ColourGradient tubeGlow (thunderforge::ThunderforgeLookAndFeel::amberGold.withAlpha ((float)glowAlpha * 0.2f),
                                   preampArea.getCentreX(), preampArea.getCentreY(),
                                   thunderforge::ThunderforgeLookAndFeel::aeroDark,
                                   preampArea.getCentreX(), preampArea.getY(), true);
    g.setGradientFill (tubeGlow);
    g.fillRoundedRectangle (preampArea, 10.0f);
    
    // LCD Refraction & Labels
    auto lcdArea = lcd.getBounds().toFloat();
    g.setColour (juce::Colours::white.withAlpha (0.03f));
    g.fillRoundedRectangle (lcdArea.removeFromTop (40), 5.0f); // Glass Glint
    
    g.setColour (thunderforge::ThunderforgeLookAndFeel::aeroCyan.withAlpha (0.6f));
    g.setFont (juce::FontOptions().withName ("JetBrains Mono").withHeight (10.0f));
    g.drawText ("AMP: " + audioProcessor.getLoadedNAMName(), lcdArea.removeFromTop (20).reduced (10, 0), juce::Justification::left);
    g.drawText ("CAB: " + audioProcessor.getLoadedIRName(), lcdArea.removeFromTop (20).reduced (10, 0), juce::Justification::left);

    // --- MODULE HEADERS ---
    // 1. Overall Chassis (Machined Metal Gradient)
    juce::ColourGradient chassis (thunderforge::ThunderforgeLookAndFeel::aeroPanelLight, 0, 0,
                                  thunderforge::ThunderforgeLookAndFeel::aeroDark, 0, bounds.getHeight(), false);
    g.setGradientFill (chassis);
    g.fillRoundedRectangle (bounds, 16.0f);
    
    // Subtle metal noise/texture could go here if we had an image, but we'll use a fine grid
    g.setColour (juce::Colours::white.withAlpha (0.02f));
    for (int i=0; i < bounds.getWidth(); i += 2)
        g.drawVerticalLine (i, 0.0f, bounds.getHeight());

    // 2. Bezel
    g.setColour (thunderforge::ThunderforgeLookAndFeel::aeroBorder);
    g.drawRoundedRectangle (bounds.reduced (1.0f), 16.0f, 2.0f);
    
    // 3. Header Text
    auto headerArea = bounds.reduced (30, 20).removeFromTop (60);
    lookAndFeel.drawHeader (g, headerArea.toNearestInt(), "AeroTone");
}

void ThunderforgeAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (30, 25);
    auto headerArea = area.removeFromTop (80);
    
    // 300x LAYOUT - METERS & HEADERS
    auto inputArea = area.removeFromLeft (30).reduced (5);
    inputMeter.setBounds (inputArea.toNearestInt());
    
    auto outputAreaR = area.removeFromRight (30).reduced (5);
    outputMeter.setBounds (outputAreaR.toNearestInt());

    // 2. Dynamics Section
    auto dynamicsArea = area.removeFromLeft (140);
    
    // Snapshot Buttons
    auto buttonsArea = area.removeFromTop (30);
    loadNAMButton.setBounds (buttonsArea.removeFromLeft (100).reduced (2));
    loadIRButton.setBounds (buttonsArea.removeFromLeft (100).reduced (2));

    // LCD Layout
    lcd.setBounds (area.removeFromTop (220).reduced (10, 0).toNearestInt());
    
    area.removeFromTop (24); // Gap
    
    // Modules Grid
    auto moduleArea = area;
    auto colW = moduleArea.getWidth() / 12;
    
    // 1. DYNAMICS Module (2 cols)
    auto dynArea = moduleArea.removeFromLeft (colW * 2).reduced (4);
    gateKnob.setBounds (dynArea.removeFromTop (dynArea.getHeight() / 2).withSizeKeepingCentre (75, 90).toNearestInt());
    compKnob.setBounds (dynArea.withSizeKeepingCentre (75, 90).toNearestInt());
    
    // 2. PREAMP Module (2 cols)
    auto preampArea = moduleArea.removeFromLeft (colW * 2).reduced (4);
    gainKnob.setBounds (preampArea.withSizeKeepingCentre (90, 110).toNearestInt());
    
    // 3. EQ Module (4 cols)
    auto eqArea = moduleArea.removeFromLeft (colW * 4).reduced (4);
    auto knobW = eqArea.getWidth() / 3;
    bassKnob.setBounds (eqArea.removeFromLeft (knobW).withSizeKeepingCentre (85, 105).toNearestInt());
    midKnob.setBounds (eqArea.removeFromLeft (knobW).withSizeKeepingCentre (85, 105).toNearestInt());
    trebleKnob.setBounds (eqArea.removeFromLeft (knobW).withSizeKeepingCentre (85, 105).toNearestInt());
    
    // 4. POST FX Module (2 cols)
    auto fxArea = moduleArea.removeFromLeft (colW * 2).reduced (4);
    delayKnob.setBounds (fxArea.removeFromTop (fxArea.getHeight() / 2).withSizeKeepingCentre (75, 90).toNearestInt());
    reverbKnob.setBounds (fxArea.withSizeKeepingCentre (75, 90).toNearestInt());
    
    // 5. OUTPUT & WIDTH Module (2 cols)
    auto outputArea = moduleArea.reduced (4);
    auto outKnobW = outputArea.getWidth() / 2;
    widthKnob.setBounds (outputArea.removeFromLeft (outKnobW).withSizeKeepingCentre (75, 90).toNearestInt());
    masterKnob.setBounds (outputArea.withSizeKeepingCentre (90, 110).toNearestInt());
    
    // 6. PRESET VAULT & TEST NOTE (Bottom Row)
    auto bottomArea = area.removeFromBottom (45);
    testNoteButton.setBounds (bottomArea.removeFromLeft (100).reduced (5).toNearestInt());
    
    // Preset Nav Group
    auto navArea = bottomArea.removeFromRight (250);
    prevButton.setBounds (navArea.removeFromLeft (40).reduced (5).toNearestInt());
    nextButton.setBounds (navArea.removeFromRight (40).reduced (5).toNearestInt());
    presetLabel.setBounds (navArea.reduced (5).toNearestInt());

    int numButtons = std::min(5, audioProcessor.getNumPresets());
    if (numButtons > 0)
    {
        auto presetW = bottomArea.getWidth() / numButtons;
        for (int i = 0; i < numButtons; ++i)
            acdcButtons[i].setBounds (bottomArea.removeFromLeft (presetW).reduced (2).toNearestInt());
    }
}

void ThunderforgeAudioProcessorEditor::timerCallback()
{
    // Update Visualizer
    std::vector<float> fftBuffer;
    float peakHz = 0.0f;
    audioProcessor.getNextFFTBlock (fftBuffer, peakHz);
    
    if (! fftBuffer.empty())
        lcd.pushNextFFTData (fftBuffer, peakHz);

    // Update Knob Glows
    float level = audioProcessor.getPeakLevel();
    gateKnob.setLevel (level);
    compKnob.setLevel (level);
    gainKnob.setLevel (level);
    bassKnob.setLevel (level);
    midKnob.setLevel (level);
    trebleKnob.setLevel (level);
    delayKnob.setLevel (level);
    reverbKnob.setLevel (level);
    widthKnob.setLevel (level);
    masterKnob.setLevel (level);

    // Update Meters
    inputMeter.setLevel (audioProcessor.getInputLevel());
    outputMeter.setLevel (audioProcessor.getOutputLevel());

    // Update Preset Label
    presetLabel.setText (audioProcessor.getPresetName (audioProcessor.getCurrentPresetIndex()), juce::dontSendNotification);
}
