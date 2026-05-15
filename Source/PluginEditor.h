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

    // FileBrowserListener
    void selectionChanged() override {}
    void fileClicked (const juce::File&, const juce::MouseEvent&) override {}
    void fileDoubleClicked (const juce::File& file) override;
    void browserRootChanged (const juce::File&) override {}

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
    juce::ComboBox namComboBox { "NAM MODEL" };
    juce::Array<juce::File> namFiles;
    juce::TextButton loadNAMButton { "LOAD AMP" };
    juce::TextButton loadIRButton { "LOAD CAB" };

    std::unique_ptr<juce::WildcardFileFilter> irFilter;
    std::unique_ptr<juce::FileBrowserComponent> irBrowser;

    class ScannerThread : public juce::Thread {
    public:
        ScannerThread(juce::File dir, std::function<void(juce::Array<juce::File>)> cb)
            : juce::Thread("Scanner"), directory(dir), callback(cb) {}

        void run() override {
            auto files = directory.findChildFiles(juce::File::findFiles, true, "*.nam");
            if (threadShouldExit()) return;

            juce::MessageManager::callAsync([cb = callback, files] {
                cb(files);
            });
        }
    private:
        juce::File directory;
        std::function<void(juce::Array<juce::File>)> callback;
    };

    std::unique_ptr<ScannerThread> scanner;
    void onScanComplete(const juce::Array<juce::File>& files);

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
