The review mentions that thunderforge::Preset is undefined, but it IS defined in Source/PluginProcessor.h!
Oh, it is defined in Source/PluginProcessor.h! The reviewer might be using a stale view or it just hallucinated. Let's check `grep -n -C 5 "struct Preset" Source/PluginProcessor.h` - yes, it's there.

The bigger issue is `callAsync` inside the audio thread. I shouldn't be using `callAsync` inside `processBlock`. Actually, what I *can* do is set the raw parameter value directly and safely update the parameter using the proper lock-free method, OR just use `param->setValueNotifyingHost(normValue);`! Wait, `param->setValueNotifyingHost()` CAN be used in `processBlock` if we do it properly, but the reviewer specifically flagged it previously:
"Calling param->setValueNotifyingHost(normValue) directly inside processBlock (the high-priority audio thread) is a severe violation of JUCE best practices...  Parameter updates from MIDI should be handled using lock-free mechanisms or JUCE's built-in parameter automation."

Wait, memory says: "When updating APVTS parameters directly from the audio thread (e.g., inside processBlock for handling MIDI CC mapping), use param->setValueNotifyingHost() to ensure the UI properly updates and the host is aware of the parameter change, rather than param->setValue()."

So memory *explicitly says* to use `param->setValueNotifyingHost()`! I should revert my `callAsync` change inside `processBlock` and go back to `param->setValueNotifyingHost()`, caching the parameter pointers in the constructor to avoid string lookups.

Let's cache the parameter pointers in the processor class.
