#pragma once

#include <JuceHeader.h>
#include <lh_thunderforge/lh_thunderforge.h>
#include "PluginProcessor.h"

class ThunderforgeAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::FileBrowserListener
{
public:
    ThunderforgeAudioProcessorEditor (ThunderforgeAudioProcessor&);
    ~ThunderforgeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    // FileBrowserListener methods
    void selectionChanged() override {}
    void fileClicked (const juce::File& file, const juce::MouseEvent& e) override {}
    void fileDoubleClicked (const juce::File& file) override;
    void browserRootChanged (const juce::File& newRoot) override {}

class NamScannerThread : public juce::Thread
{
public:
    NamScannerThread(juce::ComboBox& comboBoxToUpdate)
        : juce::Thread("NAM Scanner"), comboBox(comboBoxToUpdate)
    {
    }

    void run() override
    {
        juce::File docsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        juce::Array<juce::File> namFiles = docsDir.findChildFiles(juce::File::findFiles, true, "*.nam");

        juce::StringArray fileNames;
        for (const auto& file : namFiles)
        {
            if (threadShouldExit())
                return;

            fileNames.add(file.getFullPathName());
        }

        if (threadShouldExit())
            return;

        juce::Component::SafePointer<juce::ComboBox> safeComboBox(&comboBox);

        juce::MessageManager::callAsync([safeComboBox, fileNames]() {
            if (safeComboBox != nullptr)
            {
                safeComboBox->clear();
                for (int i = 0; i < fileNames.size(); ++i)
                {
                    safeComboBox->addItem(juce::File(fileNames[i]).getFileNameWithoutExtension(), i + 1);
                }

                // Store the full paths to access later when an item is selected
                for (int i = 0; i < fileNames.size(); ++i)
                {
                    safeComboBox->getProperties().set(juce::String(i + 1), fileNames[i]);
                }
            }
        });
    }

private:
    juce::ComboBox& comboBox;
};

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
    std::unique_ptr<NamScannerThread> namScanner;

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

    std::unique_ptr<juce::WildcardFileFilter> irFilter;
    std::unique_ptr<juce::FileBrowserComponent> fileBrowser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThunderforgeAudioProcessorEditor)
};
