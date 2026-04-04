#pragma once

#include <JuceHeader.h>

namespace thunderforge {

struct Preset {
    juce::String name;
    float drive, bass, mid, treble, presence, volume;
};

class PresetManager
{
public:
    PresetManager();
    ~PresetManager();

    int getNumPresets() const;
    juce::String getPresetName(int index) const;
    Preset getPreset(int index) const;

    void loadPresets();
    void savePresets();

private:
    void createDefaultPresets();

    std::vector<Preset> presets;
    juce::File presetFile;
};

} // namespace thunderforge
