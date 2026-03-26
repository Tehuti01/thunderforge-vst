#include "PluginProcessor.h"
#include "PluginEditor.h"

ThunderforgeAudioProcessor::ThunderforgeAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout()),
      forwardFFT (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    testOsc.setFrequency (110.0f); // A2 string
    testOsc.initialise ([] (float x) { return std::sin (x) > 0.0 ? 0.3f : -0.3f; }); // Rect for harmonics
}

ThunderforgeAudioProcessor::~ThunderforgeAudioProcessor() {}

void ThunderforgeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    noiseGate.prepare (sampleRate);
    tubeScreamer.prepare (sampleRate);
    compressor.prepare (sampleRate);
    toneStack.prepare (sampleRate);
    waveShaper.prepare (sampleRate);
    namProcessor.prepare (sampleRate, samplesPerBlock);
    cabinetSim.prepare (spec);
    delay.prepare (spec);
    reverb.prepare (spec);
    chorus.prepare (spec);
    
    testOsc.prepare (spec);
}

void ThunderforgeAudioProcessor::releaseResources() {}

bool ThunderforgeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;

    return true;
}

void ThunderforgeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update Parameters
    noiseGate.setParameters (*apvts.getRawParameterValue ("gate_threshold"),
                             *apvts.getRawParameterValue ("gate_attack"),
                             *apvts.getRawParameterValue ("gate_hold"),
                             *apvts.getRawParameterValue ("gate_release"),
                             *apvts.getRawParameterValue ("gate_bypass"));

    tubeScreamer.setParameters (*apvts.getRawParameterValue ("ts_drive"),
                                *apvts.getRawParameterValue ("ts_tone"),
                                *apvts.getRawParameterValue ("ts_level"),
                                *apvts.getRawParameterValue ("ts_bypass"));

    compressor.setParameters (*apvts.getRawParameterValue ("comp_threshold"),
                              *apvts.getRawParameterValue ("comp_ratio"),
                              *apvts.getRawParameterValue ("comp_attack"),
                              *apvts.getRawParameterValue ("comp_release"),
                              *apvts.getRawParameterValue ("comp_makeup"),
                              *apvts.getRawParameterValue ("comp_bypass"));

    toneStack.setParameters (*apvts.getRawParameterValue ("eq_bass"),
                             *apvts.getRawParameterValue ("eq_mid"),
                             *apvts.getRawParameterValue ("eq_treble"),
                             *apvts.getRawParameterValue ("eq_presence"));

    waveShaper.setParameters (*apvts.getRawParameterValue ("amp_gain"));

    delay.setParameters (*apvts.getRawParameterValue ("delay_time"),
                         *apvts.getRawParameterValue ("delay_feedback"),
                         *apvts.getRawParameterValue ("delay_mix"),
                         *apvts.getRawParameterValue ("delay_bypass"));

    reverb.setParameters (*apvts.getRawParameterValue ("reverb_size"),
                          *apvts.getRawParameterValue ("reverb_damping"),
                          *apvts.getRawParameterValue ("reverb_mix"),
                          *apvts.getRawParameterValue ("reverb_bypass"));

    chorus.setParameters (*apvts.getRawParameterValue ("chorus_rate"),
                          *apvts.getRawParameterValue ("chorus_depth"),
                          *apvts.getRawParameterValue ("chorus_mix"),
                          *apvts.getRawParameterValue ("chorus_bypass"));

    // Process Signal Chain
    if (isPlayingTestNote)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* out = buffer.getWritePointer (channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                out[sample] += testOsc.processSample (0.0f) * 0.2f;
        }
    }

    noiseGate.process (buffer);
    tubeScreamer.process (buffer);
    compressor.process (buffer);
    
    // Amp Section: NAM or fallback WaveShaper
    // For now, if nam model index > 0, we could load a model, but that needs background loading.
    // Fallback always processes if NAM is not loaded.
    waveShaper.process (buffer);
    namProcessor.process (buffer);
    
    toneStack.process (buffer);
    
    // Cabinet
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    cabinetSim.process (context);
    
    // Post FX
    delay.process (buffer);
    reverb.process (buffer);
    chorus.process (buffer);

    // Master Volume
    float masterVolDb = *apvts.getRawParameterValue ("master_volume");
    float gain = juce::Decibels::decibelsToGain (masterVolDb);
    buffer.applyGain (gain);

    // Track Peak Level for UI Glow
    float maxAbs = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        maxAbs = std::max (maxAbs, buffer.getMagnitude (channel, 0, buffer.getNumSamples()));
    peakLevel.store (maxAbs);

    // Push to FFT
    auto* channelData = buffer.getReadPointer (0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        pushNextSampleIntoFifo (channelData[i]);
}

void ThunderforgeAudioProcessor::pushNextSampleIntoFifo (float sample) noexcept
{
    if (fifoIndex == fftSize)
    {
        if (! nextFFTBlockReady)
        {
            std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);
            std::copy (fifo.begin(), fifo.end(), fftBuffer.begin());
            nextFFTBlockReady = true;
            performFFT();
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

void ThunderforgeAudioProcessor::performFFT()
{
    window.multiplyWithWindowingTable (fftBuffer.data(), fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform (fftBuffer.data());

    auto maxLevel = 0.0f;
    auto maxIndex = 0;
    
    std::vector<float> newScopeData;
    newScopeData.reserve (fftSize / 2);

    for (int i = 0; i < fftSize / 2; ++i)
    {
        auto level = juce::jmap (std::abs (fftBuffer[i]), 0.0f, (float)fftSize, 0.0f, 1.0f);
        newScopeData.push_back (level);
        
        if (level > maxLevel)
        {
            maxLevel = level;
            maxIndex = i;
        }
    }

    std::lock_guard<std::mutex> lock (fftMutex);
    scopeData = std::move (newScopeData);
    currentPeakHz = (float)maxIndex * (float)getSampleRate() / (float)fftSize;
    nextFFTBlockReady = false;
}

juce::AudioProcessorEditor* ThunderforgeAudioProcessor::createEditor()
{
    return new ThunderforgeAudioProcessorEditor (*this);
}

void ThunderforgeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ThunderforgeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ThunderforgeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Input Gain
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("input_gain", "Input Gain", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f, 1.0f), 0.0f));
    
    // Noise Gate
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gate_threshold", "Gate Threshold", juce::NormalisableRange<float> (-80.0f, 0.0f, 0.1f, 1.0f), -45.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gate_attack", "Gate Attack", 0.1f, 50.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gate_hold", "Gate Hold", 0.0f, 500.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gate_release", "Gate Release", 1.0f, 2000.0f, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gate_bypass", "Gate Bypass", false));

    // Tube Screamer
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ts_drive", "TS Drive", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ts_tone", "TS Tone", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ts_level", "TS Level", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("ts_bypass", "TS Bypass", false));

    // Compressor
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("comp_threshold", "Comp Threshold", -60.0f, 0.0f, -20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("comp_ratio", "Comp Ratio", 1.0f, 20.0f, 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("comp_attack", "Comp Attack", 0.1f, 100.0f, 10.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("comp_release", "Comp Release", 10.0f, 1000.0f, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("comp_makeup", "Comp Makeup", 0.0f, 24.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("comp_bypass", "Comp Bypass", true));

    // Amp / NAM
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("amp_gain", "Amp Gain", 0.0f, 10.0f, 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterInt> ("nam_model", "NAM Model", 0, 255, 0));

    // EQ
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eq_bass", "Bass", 0.0f, 10.0f, 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eq_mid", "Mid", 0.0f, 10.0f, 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eq_treble", "Treble", 0.0f, 10.0f, 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eq_presence", "Presence", 0.0f, 10.0f, 5.0f));

    // Cabinet
    params.push_back (std::make_unique<juce::AudioParameterInt> ("cab_model", "Cab Model", 0, 255, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cab_mix", "Cab Mix", 0.0f, 100.0f, 100.0f));

    // Delay
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay_time", "Delay Time", 1.0f, 2000.0f, 375.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay_feedback", "Delay Feedback", 0.0f, 95.0f, 40.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delay_mix", "Delay Mix", 0.0f, 100.0f, 30.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("delay_sync", "Delay Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("delay_bypass", "Delay Bypass", false));

    // Reverb
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reverb_size", "Reverb Size", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reverb_decay", "Reverb Decay", 0.1f, 10.0f, 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reverb_damping", "Reverb Damping", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reverb_predelay", "Reverb Pre-Delay", 0.0f, 200.0f, 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reverb_mix", "Reverb Mix", 0.0f, 100.0f, 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("reverb_bypass", "Reverb Bypass", false));

    // Chorus
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorus_rate", "Chorus Rate", 0.1f, 10.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorus_depth", "Chorus Depth", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorus_mix", "Chorus Mix", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("chorus_bypass", "Chorus Bypass", true));

    return { params.begin(), params.end() };
}

void ThunderforgeAudioProcessor::loadPreset (int index)
{
    struct P { float d, b, m, t, p, v; };
    static const P acdc[] = {
        { 7.5f, 5.0f, 8.5f, 5.0f, 6.0f, -4.0f }, // Back in Black
        { 6.5f, 6.0f, 7.0f, 6.0f, 5.0f, -4.0f }, // Highway to Hell
        { 8.5f, 8.0f, 6.0f, 8.0f, 7.0f, -2.0f }, // Thunderstruck
        { 4.5f, 5.0f, 8.0f, 3.0f, 2.0f, -1.0f }, // Hells Bells
        { 6.0f, 4.0f, 7.5f, 7.0f, 6.0f, -3.0f }  // Shook Me
    };

    if (index >= 0 && index < 5)
    {
        auto setParam = [this] (auto id, float val, float min, float max) {
            auto range = apvts.getParameterRange (id);
            float norm = (val - range.start) / (range.end - range.start);
            apvts.getParameter (id)->setValueNotifyingHost (norm);
        };

        setParam ("ts_drive", acdc[index].d * 10.0f, 0.0f, 100.0f);
        setParam ("eq_bass", acdc[index].b, 0.0f, 10.0f);
        setParam ("eq_mid", acdc[index].m, 0.0f, 10.0f);
        setParam ("eq_treble", acdc[index].t, 0.0f, 10.0f);
        setParam ("eq_presence", acdc[index].p, 0.0f, 10.0f);
        setParam ("master_volume", acdc[index].v, -60.0f, 12.0f);
        
        currentPresetIndex = index;
    }
}

juce::String ThunderforgeAudioProcessor::getPresetName (int index) const
{
    static const juce::String acdcNames[] = { "BACK IN BLACK", "HIGHWAY TO HELL", "THUNDERSTRUCK", "HELLS BELLS", "YOU SHOOK ME" };
    if (index >= 0 && index < 5) return acdcNames[index];
    return "USER PRESET";
}

void ThunderforgeAudioProcessor::triggerTestNote (bool play)
{
    isPlayingTestNote = play;
}

// Easy-access factory for createEditor
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ThunderforgeAudioProcessor();
}
