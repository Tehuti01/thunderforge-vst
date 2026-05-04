#pragma once

#include <JuceHeader.h>
#include "DSP/NoiseGate.h"
#include "DSP/TubeScreamer.h"
#include "DSP/Compressor.h"
#include "DSP/ToneStack.h"
#include "DSP/WaveShaper.h"
#include "DSP/NAMProcessor.h"
#include "DSP/CabinetSim.h"
#include "DSP/StereoDelay.h"
#include "DSP/PlateReverb.h"
#include "DSP/Chorus.h"

#include <queue>
#include <mutex>

namespace thunderforge {
    struct Preset {
        juce::String name;
        float drive, bass, mid, treble, presence, volume;
    };
}

class ThunderforgeAudioProcessor : public juce::AudioProcessor
{
public:
    ThunderforgeAudioProcessor();
    ~ThunderforgeAudioProcessor() override;

    // ... (standard methods)
    void initializePresets();
    void loadPreset (int index);
    void triggerTestNote (bool play);
    
    float getPeakLevel() const noexcept { return peakLevel.load(); }
    int getCurrentPresetIndex() const noexcept { return currentPresetIndex; }
    juce::String getPresetName (int i) const;
    std::vector<thunderforge::Preset> presets;
    float getInputLevel() const noexcept { return inputLevel.load(); }
    float getOutputLevel() const noexcept { return outputLevel.load(); }

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    // 300x SNAPSHOT LOADING
    void loadNAMModel (const juce::File& file);
    void loadCabinetIR (const juce::File& file);
    juce::String getLoadedNAMName() const { return currentNAMName; }
    juce::String getLoadedIRName() const { return currentIRName; }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // DSP Modules
    thunderforge::NoiseGate noiseGate;
    thunderforge::TubeScreamer tubeScreamer;
    thunderforge::Compressor compressor;
    thunderforge::ToneStack toneStack;
    thunderforge::WaveShaper waveShaper;
    thunderforge::NAMProcessor namProcessor;
    thunderforge::CabinetSim cabinetSim;
    thunderforge::StereoDelay delay;
    thunderforge::PlateReverb reverb;
    thunderforge::Chorus chorus;

    // FFT / Metering
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftBuffer;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    std::vector<float> scopeData;
    std::atomic<float> currentPeakHz { 0.0f };
    std::atomic<float> peakLevel { 0.0f };
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };
    
    // 300x Modules
    std::unique_ptr<nam::DSP> namModule;
    juce::dsp::Convolution cabinetIR;
    juce::String currentNAMName { "ELITE TUBE" };
    juce::String currentIRName { "4x12 V30" };

    std::mutex fftMutex;
    
    // Internal Test Oscillator / DSP Helpers
    juce::dsp::Oscillator<float> testOsc;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    
    bool isPlayingTestNote = false;
    int currentPresetIndex = 0;
    
    void pushNextSampleIntoFifo (float sample) noexcept;
    void performFFT();

    juce::MidiKeyboardState keyboardState;

public:
    void getNextFFTBlock (std::vector<float>& destData, float& peakHz)
    {
        std::lock_guard<std::mutex> lock (fftMutex);
        destData = scopeData;
        peakHz = currentPeakHz;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThunderforgeAudioProcessor)
};
