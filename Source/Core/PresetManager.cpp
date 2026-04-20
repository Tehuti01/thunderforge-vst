#include "PresetManager.h"

namespace thunderforge {

PresetManager::PresetManager()
{
    loadPresets();
    if (presets.empty()) {
        // Fallback default presets if file didn't exist or was empty
        presets = {
            { "BACK IN BLACK", 7.5f, 5.0f, 8.5f, 5.0f, 6.0f, -4.0f },
            { "HIGHWAY TO HELL", 6.5f, 6.0f, 7.0f, 6.0f, 5.0f, -4.0f },
            { "THUNDERSTRUCK", 8.5f, 8.0f, 6.0f, 8.0f, 7.0f, -2.0f },
            { "HELLS BELLS", 4.5f, 5.0f, 8.0f, 3.0f, 2.0f, -1.0f },
            { "SHOOK ME", 6.0f, 4.0f, 7.5f, 7.0f, 6.0f, -3.0f }
        };
        savePresets();
    }
}

juce::File PresetManager::getPresetsFile() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("LukasHansenAudio")
        .getChildFile("LHThunderforge");

    if (!dir.exists()) {
        dir.createDirectory();
    }
    return dir.getChildFile("presets.json");
}

void PresetManager::loadPresets()
{
    presets.clear();
    auto file = getPresetsFile();
    if (!file.existsAsFile()) return;

    juce::var parsedJson = juce::JSON::parse(file);
    if (parsedJson.isArray())
    {
        auto* array = parsedJson.getArray();
        for (auto& item : *array)
        {
            if (item.isObject())
            {
                Preset p;
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

void PresetManager::savePresets()
{
    juce::var array(juce::Array<juce::var>{});

    for (const auto& p : presets)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", p.name);
        obj->setProperty("drive", p.drive);
        obj->setProperty("bass", p.bass);
        obj->setProperty("mid", p.mid);
        obj->setProperty("treble", p.treble);
        obj->setProperty("presence", p.presence);
        obj->setProperty("volume", p.volume);
        array.append(juce::var(obj));
    }

    auto file = getPresetsFile();
    auto jsonString = juce::JSON::toString(array);
    file.replaceWithText(jsonString);
}

int PresetManager::getNumPresets() const
{
    return (int)presets.size();
}

Preset PresetManager::getPreset(int index) const
{
    if (index >= 0 && index < getNumPresets())
        return presets[(size_t)index];
    return {};
}

void PresetManager::addPreset(const Preset& preset)
{
    presets.push_back(preset);
    savePresets();
}

void PresetManager::updatePreset(int index, const Preset& preset)
{
    if (index >= 0 && index < getNumPresets()) {
        presets[(size_t)index] = preset;
        savePresets();
    }
}

} // namespace thunderforge
