1. **Fix Master Volume Parameter and Crash Issues**
   - Update `PluginProcessor.cpp` to remove unused references to `master_volume`.
   - Update `processBlock` to properly handle `master_volume` and `bypass_cabinet` parameters by either removing them or adding them to the parameter layout. We will remove the undefined parameters and use `output_gain` instead as per the memory instructions.

2. **Implement JSON Preset Manager**
   - Add `loadPresetsFromJSON` and `savePresetsToJSON` methods to `ThunderforgeAudioProcessor` to read and write presets from/to `juce::File::userApplicationDataDirectory/AeroTone/Thunderforge/presets.json`.
   - Replace the hardcoded ACDC array in `loadPreset` and `getPresetName` with the dynamically loaded JSON array.
   - Explicitly cast extracted numeric values to `double` before `float` when parsing JSON, as requested in the memory.

3. **Update UI**
   - In `PluginEditor.cpp`, update the quick-access preset buttons to use dynamically loaded preset names.
   - Update the `<` / `>` buttons in `PluginEditor.cpp` to correctly navigate through any number of loaded presets, bounded by `getNumPresets()`.

4. **Verify Build and Test**
   - Run `cmake -B build` and `cmake --build build --target Thunderforge_Standalone -j 8`.
   - Verify it runs using headless execution with `xvfb-run`.

5. **Pre-commit Steps**
   - Execute pre-commit instructions to ensure proper testing, verifications, reviews, and reflections are done.

6. **Submit Code**
   - Commit and submit changes once testing is verified.
