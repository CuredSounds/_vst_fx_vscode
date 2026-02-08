cmake_minimum_required(VERSION 3.15)

project(BC_COMPRESSOR VERSION 1.0.0)

# macOS 15 compatibility: Target macOS 14.0 to avoid unavailable APIs
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS deployment version")

# Workaround for macOS 15 deprecated API warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations -Wno-error=unavailable-declarations")

# Add JUCE to the project
add_subdirectory(JUCE-master)

# Create the plugin target
juce_add_plugin(BCCompressor
    COMPANY_NAME "CuredSounds"
    PLUGIN_MANUFACTURER_CODE Curs
    PLUGIN_CODE BCmp
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "BC Compressor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES Fx Dynamics
)

# Generate JuceHeader
juce_generate_juce_header(BCCompressor)

# Source files
target_sources(BCCompressor
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DSP/EffectModule.cpp
        Source/DSP/CompressorModule.cpp
        Source/DSP/DelayModule.cpp
        Source/DSP/SaturationModule.cpp
        Source/DSP/EffectChain.cpp
)

# Preprocessor definitions
target_compile_definitions(BCCompressor
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link JUCE modules
target_link_libraries(BCCompressor
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
