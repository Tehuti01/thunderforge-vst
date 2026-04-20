#pragma once
#include <JuceHeader.h>
#include <vector>

namespace thunderforge {

struct Preset {
    juce::String name;
    float drive { 0.0f };
    float bass { 5.0f };
    float mid { 5.0f };
    float treble { 5.0f };
    float presence { 5.0f };
    float volume { 0.0f };
};

class PresetManager
{
public:
    PresetManager();
    ~PresetManager() = default;

    void loadPresets();
    void savePresets();

    int getNumPresets() const;
    Preset getPreset(int index) const;
    void addPreset(const Preset& preset);
    void updatePreset(int index, const Preset& preset);

private:
    std::vector<Preset> presets;
    juce::File getPresetsFile() const;
};

} // namespace thunderforge
