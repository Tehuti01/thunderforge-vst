#pragma once

#include <JuceHeader.h>
#include <lh_thunderforge/lh_thunderforge.h>
#include "PluginProcessor.h"

class ThunderforgeAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    ThunderforgeAudioProcessorEditor (ThunderforgeAudioProcessor&);
    ~ThunderforgeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    ThunderforgeAudioProcessor& audioProcessor;

    thunderforge::ThunderforgeLookAndFeel lookAndFeel;

    thunderforge::KnobComponent gateKnob    { "GATE" };
    thunderforge::KnobComponent compKnob    { "COMP" };
    thunderforge::KnobComponent gainKnob    { "GAIN" };
    thunderforge::KnobComponent bassKnob    { "BASS" };
    thunderforge::KnobComponent midKnob     { "MID" };
    thunderforge::KnobComponent trebleKnob  { "TREBLE" };
    thunderforge::KnobComponent delayKnob   { "DELAY" };
    thunderforge::KnobComponent reverbKnob { "Reverb" };
    thunderforge::KnobComponent masterKnob { "Output" };
    thunderforge::KnobComponent widthKnob  { "Width" };

    thunderforge::LCDComponent lcd;
    
    juce::TextButton acdcButtons[5];
    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };
    juce::Label presetLabel;
    juce::TextButton testNoteButton { "TEST NOTE" };
    juce::ComboBox namComboBox;
    juce::ComboBox irComboBox;

    juce::StringArray namFiles;
    juce::StringArray irFiles;

    // 300x Metrics
    thunderforge::VU_Meter inputMeter;
    thunderforge::VU_Meter outputMeter;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> gateAttachment;
    std::unique_ptr<Attachment> compAttachment;
    std::unique_ptr<Attachment> gainAttachment;
    std::unique_ptr<Attachment> bassAttachment;
    std::unique_ptr<Attachment> midAttachment;
    std::unique_ptr<Attachment> trebleAttachment;
    std::unique_ptr<Attachment> delayAttachment;
    std::unique_ptr<Attachment> reverbAttachment;
    std::unique_ptr<Attachment> masterAttachment;
    std::unique_ptr<Attachment> widthAttachment;
    
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThunderforgeAudioProcessorEditor)
};
