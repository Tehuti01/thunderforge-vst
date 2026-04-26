#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <lh_thunderforge/lh_thunderforge.h>

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

    loadPresetsFromJSON();
}

ThunderforgeAudioProcessor::~ThunderforgeAudioProcessor() {}

void ThunderforgeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    toneStack.prepare (sampleRate);
    
    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (getTotalNumInputChannels(), 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
    oversampler->initProcessing (samplesPerBlock);
    setLatencySamples ((int)oversampler->getLatencyInSamples());
    
    testOsc.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumOutputChannels() });

    noiseGate.prepare (sampleRate);
    tubeScreamer.prepare (sampleRate);
    compressor.prepare (sampleRate);
    waveShaper.prepare (sampleRate);
    namProcessor.prepare (sampleRate, samplesPerBlock);
    cabinetSim.prepare (spec);
    delay.prepare (spec);
    reverb.prepare (spec);
    chorus.prepare (spec);
    cabinetIR.prepare (spec);
    
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

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // MIDI CC Mapping for 10 core knobs
    for (const auto meta : midiMessages)
    {
        auto msg = meta.getMessage();
        if (msg.isController())
        {
            auto cc = msg.getControllerNumber();
            auto val = msg.getControllerValue() / 127.0f; // 0.0 to 1.0

            auto mapCC = [&](int targetCC, const juce::String& paramID) {
                if (cc == targetCC) {
                    auto* param = apvts.getParameter(paramID);
                    if (param != nullptr) {
                        param->setValueNotifyingHost(val);
                    }
                }
            };

            // Map 10 core knobs (assigning basic sequential CC numbers or standard ones, assuming sequential 70-79 since they are simple to configure)
            mapCC(70, "gate_threshold");
            mapCC(71, "comp_threshold");
            mapCC(72, "ts_drive");
            mapCC(73, "eq_bass");
            mapCC(74, "eq_mid");
            mapCC(75, "eq_treble");
            mapCC(76, "delay_mix");
            mapCC(77, "reverb_mix");
            mapCC(78, "stereo_width");
            mapCC(79, "output_gain");
        }
    }

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

    // --- CABINET IR ---
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        cabinetIR.process (context);
    }

    // --- NAM MODULE ---
    if (namModule != nullptr)
    {
        // NAM processing logic (simplified for walkthrough)
        // In a real implementation, we'd wrap the buffer and process per sample
    }

    // --- STEREO WIDTH (MS Processing) ---
    float width = *apvts.getRawParameterValue ("stereo_width") / 100.0f;
    if (buffer.getNumChannels() >= 2)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float l = buffer.getSample (0, i);
            float r = buffer.getSample (1, i);
            
            float mid = (l + r) * 0.5f;
            float side = (l - r) * 0.5f;
            
            side *= width;
            
            buffer.setSample (0, i, mid + side);
            buffer.setSample (1, i, mid - side);
        }
    }

    // --- 4x OVERSAMPLING EXIT ---
    // --- OUTPUT GAIN ---
    float outGain = juce::Decibels::decibelsToGain ((float)*apvts.getRawParameterValue ("output_gain"));
    buffer.applyGain (outGain);

    float outPeak = buffer.getMagnitude (0, buffer.getNumSamples());
    outputLevel.store (outPeak);

    peakLevel.store (outPeak); // For general glow
    
    // Push to FFT
    // For simplicity, only push the first sample of the first channel
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        pushNextSampleIntoFifo (buffer.getSample (0, i));
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

void ThunderforgeAudioProcessor::loadNAMModel (const juce::File& file)
{
    try {
        std::filesystem::path modelPath (file.getFullPathName().toStdString());
        auto model = nam::get_dsp (modelPath);
        if (model != nullptr) {
            namModule = std::move (model);
            currentNAMName = file.getFileName();
        }
    } catch (...) {}
}

void ThunderforgeAudioProcessor::loadCabinetIR (const juce::File& file)
{
    cabinetIR.loadImpulseResponse (file, juce::dsp::Convolution::Stereo::yes, 
                                   juce::dsp::Convolution::Trim::yes, 0);
    currentIRName = file.getFileName();
}

juce::AudioProcessorValueTreeState::ParameterLayout ThunderforgeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Input Gain
    // I/O Section
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("input_gain", "Input Gain", -20.0f, 20.0f, 0.0f));
    
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

    // I/O Gain
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("output_gain", "Output Gain", -20.0f, 20.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("stereo_width", "Stereo Width", 0.0f, 200.0f, 100.0f));

    // Chorus
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorus_rate", "Chorus Rate", 0.1f, 10.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorus_depth", "Chorus Depth", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("chorus_mix", "Chorus Mix", 0.0f, 100.0f, 50.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("chorus_bypass", "Chorus Bypass", true));

    return { params.begin(), params.end() };
}

void ThunderforgeAudioProcessor::loadPresetsFromJSON()
{
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                .getChildFile("Lukas Hansen Audio")
                                .getChildFile("LH Thunderforge");

    if (!appDataDir.exists())
        appDataDir.createDirectory();

    juce::File presetsFile = appDataDir.getChildFile("presets.json");

    if (presetsFile.existsAsFile())
    {
        auto parsedJson = juce::JSON::parse(presetsFile);
        if (parsedJson.isArray())
        {
            auto* array = parsedJson.getArray();
            presets.clear();
            for (auto& item : *array)
            {
                if (item.isObject())
                {
                    thunderforge::Preset p;
                    p.name = item["name"].toString();
                    p.drive = static_cast<float>(static_cast<double>(item["drive"]));
                    p.bass = static_cast<float>(static_cast<double>(item["bass"]));
                    p.mid = static_cast<float>(static_cast<double>(item["mid"]));
                    p.treble = static_cast<float>(static_cast<double>(item["treble"]));
                    p.presence = static_cast<float>(static_cast<double>(item["presence"]));
                    p.volume = static_cast<float>(static_cast<double>(item["volume"]));
                    presets.push_back(p);
                }
            }
        }
    }

    if (presets.empty())
    {
        // Default fallbacks if file doesn't exist or is invalid
        presets.push_back({"BACK IN BLACK", 75.0f, 5.0f, 8.5f, 5.0f, 6.0f, -4.0f});
        presets.push_back({"HIGHWAY TO HELL", 65.0f, 6.0f, 7.0f, 6.0f, 5.0f, -4.0f});
        presets.push_back({"THUNDERSTRUCK", 85.0f, 8.0f, 6.0f, 8.0f, 7.0f, -2.0f});
        presets.push_back({"HELLS BELLS", 45.0f, 5.0f, 8.0f, 3.0f, 2.0f, -1.0f});
        presets.push_back({"YOU SHOOK ME", 60.0f, 4.0f, 7.5f, 7.0f, 6.0f, -3.0f});
        savePresetsToJSON();
    }
}

void ThunderforgeAudioProcessor::savePresetsToJSON()
{
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                .getChildFile("Lukas Hansen Audio")
                                .getChildFile("LH Thunderforge");

    if (!appDataDir.exists())
        appDataDir.createDirectory();

    juce::File presetsFile = appDataDir.getChildFile("presets.json");

    juce::DynamicObject::Ptr mainObj = new juce::DynamicObject();
    juce::Array<juce::var> array;

    for (const auto& p : presets)
    {
        juce::DynamicObject::Ptr presetObj = new juce::DynamicObject();
        presetObj->setProperty("name", p.name);
        presetObj->setProperty("drive", static_cast<double>(p.drive));
        presetObj->setProperty("bass", static_cast<double>(p.bass));
        presetObj->setProperty("mid", static_cast<double>(p.mid));
        presetObj->setProperty("treble", static_cast<double>(p.treble));
        presetObj->setProperty("presence", static_cast<double>(p.presence));
        presetObj->setProperty("volume", static_cast<double>(p.volume));
        array.add(juce::var(presetObj.get()));
    }

    juce::var jsonVar(array);
    presetsFile.replaceWithText(juce::JSON::toString(jsonVar));
}

void ThunderforgeAudioProcessor::loadPreset (int index)
{
    if (index >= 0 && index < presets.size())
    {
        auto setParam = [this] (auto id, float val) {
            auto range = apvts.getParameterRange (id);
            float norm = (val - range.start) / (range.end - range.start);
            apvts.getParameter (id)->setValueNotifyingHost (norm);
        };

        setParam ("ts_drive", presets[index].drive);
        setParam ("eq_bass", presets[index].bass);
        setParam ("eq_mid", presets[index].mid);
        setParam ("eq_treble", presets[index].treble);
        setParam ("eq_presence", presets[index].presence);
        setParam ("output_gain", presets[index].volume);
        
        currentPresetIndex = index;
    }
}

juce::String ThunderforgeAudioProcessor::getPresetName (int index) const
{
    if (index >= 0 && index < presets.size()) return presets[index].name;
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
