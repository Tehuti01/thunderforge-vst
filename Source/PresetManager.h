#pragma once

#include <JuceHeader.h>
#include <vector>

namespace thunderforge {

struct Preset {
    juce::String name;
    juce::String shortName;
    float drive, bass, mid, treble, presence, volume;
};

class PresetManager
{
public:
    PresetManager();

    void loadPresetsFromJson (const juce::String& jsonString);

    int getNumPresets() const { return (int)presets.size(); }
    const Preset& getPreset (int index) const;

private:
    std::vector<Preset> presets;
    Preset fallbackPreset { "USER PRESET", "USER", 50.0f, 5.0f, 5.0f, 5.0f, 5.0f, 0.0f };
};

} // namespace thunderforge
