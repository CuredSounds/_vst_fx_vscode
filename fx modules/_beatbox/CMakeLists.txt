cmake_minimum_required(VERSION 3.22)
project(_BC_BeatBox VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE framework
add_subdirectory(JUCE)

# Plugin formats to build
set(FORMATS VST3 AU Standalone)

# Plugin configuration
juce_add_plugin(BeatBox
    COMPANY_NAME "Sonicdesign"
    PLUGIN_MANUFACTURER_CODE SncD
    PLUGIN_CODE BtBx
    FORMATS ${FORMATS}
    PRODUCT_NAME "BeatBox"

    # Plugin characteristics
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE

    # Bundle/package info
    BUNDLE_ID com.sonicdesign.beatbox

    # Copy plugin after build (macOS)
    COPY_PLUGIN_AFTER_BUILD TRUE

    # Version
    VERSION ${PROJECT_VERSION}
    COMPANY_WEBSITE "https://sonicdesign.com"
    COMPANY_EMAIL "info@sonicdesign.com"
    PLUGIN_MANUFACTURER_CODE SncD
    PLUGIN_CODE BtBx
)

# Source files
target_sources(BeatBox
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Audio/DrumSynthEngine.cpp
        Source/Audio/DrumSampler.cpp
        Source/Audio/DrumVoice.cpp
        Source/Audio/AudioEngine.cpp
        Source/GUI/DrumPadComponent.cpp
        Source/GUI/SequencerComponent.cpp
        Source/Utils/Logger.cpp
)

# Binary data (samples, assets, etc.)
juce_add_binary_data(BeatBoxData
    SOURCES
        Resources/.gitkeep
)

# Link dependencies
target_link_libraries(BeatBox
    PRIVATE
        BeatBoxData
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Preprocessor definitions
target_compile_definitions(BeatBox
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_USE_FLAC=1
        JUCE_USE_OGGVORBIS=1
        BEATBOX_VERSION="${PROJECT_VERSION}"
)

# Include directories
target_include_directories(BeatBox
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
