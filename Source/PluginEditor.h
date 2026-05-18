#pragma once

#include <JuceHeader.h>
#include <lh_thunderforge/lh_thunderforge.h>
#include "PluginProcessor.h"

class NAMScannerThread : public juce::Thread
{
public:
    NAMScannerThread (juce::ComboBox& cb) : juce::Thread ("NAM Scanner Thread"), comboBox (cb) {}

    void run() override
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
        juce::DirectoryIterator iter (dir, true, "*.nam");

        juce::StringArray names;

        while (iter.next())
        {
            if (threadShouldExit())
                return;

            auto f = iter.getFile();
            names.add (f.getFileName());
            scannedFiles.add (f);
        }

        juce::Component::SafePointer<juce::ComboBox> safeBox (&comboBox);
        juce::MessageManager::callAsync ([safeBox, names] {
            if (safeBox != nullptr)
            {
                safeBox->clear();
                safeBox->addItemList (names, 1);
            }
        });
    }

    juce::Array<juce::File> scannedFiles;

private:
    juce::ComboBox& comboBox;
};

class ThunderforgeAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::ComboBox::Listener
{
public:
    ThunderforgeAudioProcessorEditor (ThunderforgeAudioProcessor&);
    ~ThunderforgeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;

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
    juce::TextButton loadNAMButton { "LOAD AMP" };
    juce::TextButton loadIRButton { "LOAD CAB" };

    juce::ComboBox namSelector;
    std::unique_ptr<NAMScannerThread> namScanner;

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
