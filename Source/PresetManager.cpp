#include "PresetManager.h"

namespace thunderforge {

PresetManager::PresetManager()
{
    // Default AC/DC Vault
    juce::String defaultJson = R"({
        "presets": [
            { "name": "BACK IN BLACK", "shortName": "BACK IN BLACK", "drive": 75.0, "bass": 5.0, "mid": 8.5, "treble": 5.0, "presence": 6.0, "volume": -4.0 },
            { "name": "HIGHWAY TO HELL", "shortName": "HIGHWAY", "drive": 65.0, "bass": 6.0, "mid": 7.0, "treble": 6.0, "presence": 5.0, "volume": -4.0 },
            { "name": "THUNDERSTRUCK", "shortName": "THUNDER", "drive": 85.0, "bass": 8.0, "mid": 6.0, "treble": 8.0, "presence": 7.0, "volume": -2.0 },
            { "name": "HELLS BELLS", "shortName": "HELLS BELLS", "drive": 45.0, "bass": 5.0, "mid": 8.0, "treble": 3.0, "presence": 2.0, "volume": -1.0 },
            { "name": "YOU SHOOK ME", "shortName": "SHOOK ME", "drive": 60.0, "bass": 4.0, "mid": 7.5, "treble": 7.0, "presence": 6.0, "volume": -3.0 }
        ]
    })";

    loadPresetsFromJson (defaultJson);
}

void PresetManager::loadPresetsFromJson (const juce::String& jsonString)
{
    juce::var parsedJson;
    juce::Result result = juce::JSON::parse (jsonString, parsedJson);

    if (result.wasOk() && parsedJson.isObject())
    {
        auto* obj = parsedJson.getDynamicObject();
        if (obj->hasProperty ("presets"))
        {
            auto presetsArray = obj->getProperty ("presets");
            if (presetsArray.isArray())
            {
                presets.clear();
                for (int i = 0; i < presetsArray.size(); ++i)
                {
                    auto presetVar = presetsArray[i];
                    if (presetVar.isObject())
                    {
                        Preset p;
                        p.name = presetVar.getProperty ("name", "Unknown").toString();
                        p.shortName = presetVar.getProperty ("shortName", "Unk").toString();

                        // Use the robust casting mentioned in memory
                        p.drive    = static_cast<float>(static_cast<double>(presetVar.getProperty ("drive", 50.0)));
                        p.bass     = static_cast<float>(static_cast<double>(presetVar.getProperty ("bass", 5.0)));
                        p.mid      = static_cast<float>(static_cast<double>(presetVar.getProperty ("mid", 5.0)));
                        p.treble   = static_cast<float>(static_cast<double>(presetVar.getProperty ("treble", 5.0)));
                        p.presence = static_cast<float>(static_cast<double>(presetVar.getProperty ("presence", 5.0)));
                        p.volume   = static_cast<float>(static_cast<double>(presetVar.getProperty ("volume", 0.0)));

                        presets.push_back (p);
                    }
                }
            }
        }
    }
}

const Preset& PresetManager::getPreset (int index) const
{
    if (index >= 0 && index < (int)presets.size())
        return presets[(size_t)index];
    return fallbackPreset;
}

} // namespace thunderforge
