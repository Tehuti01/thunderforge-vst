cmake_minimum_required(VERSION 3.22)
project(Thunderforge VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE framework
add_subdirectory(JUCE)

# NeuralAmpModelerCore
# NeuralAmpModelerCore uses CPM so it might download Eigen itself.
add_subdirectory(modules/NeuralAmpModelerCore)

juce_add_plugin(Thunderforge
    COMPANY_NAME "Lukas Hansen Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    BUNDLE_ID com.LukasHansen.LHThunderforge
    BUNDLE_NAME "LH Thunderforge"
    PLUGIN_MANUFACTURER_CODE LkHn
    PLUGIN_CODE Thfg
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "LH Thunderforge"
    VST3_CATEGORIES "Fx|Distortion|Guitar"
)

# Custom Thunderforge Module
juce_add_module(modules/lh_thunderforge)

# NeuralAmpModelerCore as a library
file(GLOB NAM_SOURCES 
    modules/NeuralAmpModelerCore/NAM/*.cpp
)

add_library(NAM_Core STATIC ${NAM_SOURCES})
target_compile_features(NAM_Core PUBLIC cxx_std_20)
target_include_directories(NAM_Core PUBLIC 
    modules/NeuralAmpModelerCore
    modules/NeuralAmpModelerCore/NAM
    modules/NeuralAmpModelerCore/Dependencies/eigen
    modules/NeuralAmpModelerCore/Dependencies/AudioDSPTools/Dependencies/nlohmann
)

# Main Plugin sources
target_sources(Thunderforge PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(Thunderforge PRIVATE
    Source
)

target_link_libraries(Thunderforge PRIVATE curl
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_opengl
    lh_thunderforge
    NAM_Core
)

target_compile_definitions(Thunderforge PRIVATE
    JUCE_VST2_CAN_REPLACE_VST3=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

juce_generate_juce_header(Thunderforge)
