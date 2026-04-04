#include "PresetManager.h"

namespace thunderforge {

PresetManager::PresetManager()
{
    auto userDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    presetFile = userDir.getChildFile("LukasHansenAudio").getChildFile("Thunderforge").getChildFile("presets.json");

    loadPresets();
}

PresetManager::~PresetManager()
{
    // Optionally save on exit if modifications were allowed
    // savePresets();
}

int PresetManager::getNumPresets() const
{
    return (int)presets.size();
}

juce::String PresetManager::getPresetName(int index) const
{
    if (index >= 0 && index < (int)presets.size())
        return presets[(size_t)index].name;
    return "USER PRESET";
}

Preset PresetManager::getPreset(int index) const
{
    if (index >= 0 && index < (int)presets.size())
        return presets[(size_t)index];

    return { "DEFAULT", 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 0.0f };
}

void PresetManager::loadPresets()
{
    if (!presetFile.existsAsFile())
    {
        createDefaultPresets();
        savePresets();
        return;
    }

    juce::var parsedJson = juce::JSON::parse(presetFile);

    if (!parsedJson.isVoid() && parsedJson.isArray())
    {
        presets.clear();
        auto array = parsedJson.getArray();
        for (auto& item : *array)
        {
            if (item.isObject())
            {
                auto* obj = item.getDynamicObject();
                Preset p;
                p.name = obj->getProperty("name").toString();
                p.drive = static_cast<float>(static_cast<double>(obj->getProperty("drive")));
                p.bass = static_cast<float>(static_cast<double>(obj->getProperty("bass")));
                p.mid = static_cast<float>(static_cast<double>(obj->getProperty("mid")));
                p.treble = static_cast<float>(static_cast<double>(obj->getProperty("treble")));
                p.presence = static_cast<float>(static_cast<double>(obj->getProperty("presence")));
                p.volume = static_cast<float>(static_cast<double>(obj->getProperty("volume")));
                presets.push_back(p);
            }
        }
    }

    if (presets.empty())
    {
         createDefaultPresets();
    }
}

void PresetManager::savePresets()
{
    juce::Array<juce::var> presetArray;

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

        presetArray.add(juce::var(obj));
    }

    juce::var mainObj(presetArray);

    presetFile.getParentDirectory().createDirectory(); // Ensure the directory exists

    juce::FileOutputStream stream(presetFile);
    if (stream.openedOk())
    {
        stream.setPosition(0);
        stream.truncate();
        juce::JSON::writeToStream(stream, mainObj);
    }
}

void PresetManager::createDefaultPresets()
{
    presets.clear();
    presets.push_back({ "BACK IN BLACK", 7.5f, 5.0f, 8.5f, 5.0f, 6.0f, -4.0f });
    presets.push_back({ "HIGHWAY TO HELL", 6.5f, 6.0f, 7.0f, 6.0f, 5.0f, -4.0f });
    presets.push_back({ "THUNDERSTRUCK", 8.5f, 8.0f, 6.0f, 8.0f, 7.0f, -2.0f });
    presets.push_back({ "HELLS BELLS", 4.5f, 5.0f, 8.0f, 3.0f, 2.0f, -1.0f });
    presets.push_back({ "YOU SHOOK ME", 6.0f, 4.0f, 7.5f, 7.0f, 6.0f, -3.0f });
}

} // namespace thunderforge
