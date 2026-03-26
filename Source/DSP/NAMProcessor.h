#pragma once

#include <filesystem>
#include <JuceHeader.h>
#include "../../modules/NeuralAmpModelerCore/NAM/get_dsp.h"

namespace thunderforge
{

class NAMProcessor
{
public:
    NAMProcessor() {}

    void prepare (double sampleRate, int maxBlockSize)
    {
        fs = sampleRate;
        this->maxBlockSize = maxBlockSize;
    }

    bool loadModel (const juce::File& modelFile)
    {
        try {
            auto modelPath = std::filesystem::path (modelFile.getFullPathName().toStdString());
            currentModel = nam::get_dsp (modelPath);
            return currentModel != nullptr;
        } catch (...) {
            return false;
        }
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (currentModel == nullptr) return;

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        // NAM is mono, so we process one channel and copy to others if needed
        // Or process separately if we want stereo (but guitar is usually mono)
        auto* channelData = buffer.getWritePointer (0);
        
        // Convert to double for NAM if necessary, or check if nam handles float
        // NAM usually works on blocks
        std::vector<double> input (numSamples);
        std::vector<double> output (numSamples);

        for (int i = 0; i < numSamples; ++i)
            input[i] = static_cast<double> (channelData[i]);

        double* inPtr = input.data();
        double* outPtr = output.data();
        currentModel->process (&inPtr, &outPtr, numSamples);

        for (int i = 0; i < numSamples; ++i)
            channelData[i] = static_cast<float> (output[i]);

        // If stereo, copy to second channel
        if (numChannels > 1)
            buffer.copyFrom (1, 0, buffer.getReadPointer (0), numSamples);
    }

private:
    double fs = 44100.0;
    int maxBlockSize = 512;
    std::unique_ptr<nam::DSP> currentModel;
};

} // namespace thunderforge
