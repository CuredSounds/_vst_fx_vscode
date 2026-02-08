# BC BeatBox - Code Snippets & Terminal Commands

Quick reference for replicating the build and making modifications.

---

## Terminal Commands

### Initial Setup
```bash
# Navigate to project
cd /Users/sonic.design/CLionProjects/_BC_BeatBox

# Clone JUCE (if not already done)
git clone https://github.com/juce-framework/JUCE.git

# Create directory structure
mkdir -p Source/Audio Source/GUI Source/Utils Resources Samples Tests docs
touch Resources/.gitkeep Samples/.gitkeep
```

### CMake Configuration
```bash
# Configure project
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Or for Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

### Building
```bash
# Build all formats (VST3, AU, Standalone)
cmake --build build --config Release -j8

# Clean build
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j8

# Build specific target
cmake --build build --target BeatBox_VST3 --config Release
```

### Installation
```bash
# Install VST3
cp -r "build/BeatBox_artefacts/Release/VST3/BC BeatBox.vst3" \
      ~/Library/Audio/Plug-Ins/VST3/

# Install AU
cp -r "build/BeatBox_artefacts/Release/AU/BC BeatBox.component" \
      ~/Library/Audio/Plug-Ins/Components/

# Reset Audio Component cache (macOS)
killall -9 AudioComponentRegistrar

# Install Standalone App
cp -r "build/BeatBox_artefacts/Release/Standalone/BC BeatBox.app" \
      /Applications/
```

### Viewing Logs
```bash
# Tail live logs
tail -f ~/Library/Logs/BeatBox/beatbox.log

# View full log
cat ~/Library/Logs/BeatBox/beatbox.log

# Clear logs
rm ~/Library/Logs/BeatBox/beatbox.log
```

### Finding Built Artifacts
```bash
# List all built plugins
find build -name "*.vst3" -o -name "*.component" -o -name "*.app"

# Check build status
ls -lh build/BeatBox_artefacts/Release/
```

---

## CMakeLists.txt (Complete)

```cmake
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
    COMPANY_NAME "BroadcastConcepts"
    PLUGIN_MANUFACTURER_CODE BrCo
    PLUGIN_CODE BtBx
    FORMATS ${FORMATS}
    PRODUCT_NAME "BC BeatBox"

    # Plugin characteristics
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE

    # Bundle/package info
    BUNDLE_ID com.broadcastconcepts.beatbox

    # Copy plugin after build (macOS)
    COPY_PLUGIN_AFTER_BUILD TRUE

    # Version
    VERSION ${PROJECT_VERSION}
    COMPANY_WEBSITE "https://broadcastconcepts.com"
    COMPANY_EMAIL "info@broadcastconcepts.com"
    PLUGIN_MANUFACTURER_CODE BrCo
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
```

---

## Key Code Patterns

### JUCE Module Includes (Not JuceHeader.h!)

```cpp
// Audio processing
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_formats/juce_audio_formats.h>

// GUI
#include <juce_gui_basics/juce_gui_basics.h>

// Core utilities
#include <juce_core/juce_core.h>
```

### Logger Usage

```cpp
#include "Utils/Logger.h"

// Initialization (automatic in singleton)
auto& logger = BeatBox::Logger::getInstance();

// Logging macros
LOG_DEBUG("Detailed debug information");
LOG_INFO("General information");
LOG_WARNING("Warning: potential issue");
LOG_ERROR("Error occurred");
LOG_CRITICAL("Critical failure");

// With variables
LOG_INFO("Sample rate: " + juce::String(sampleRate));
LOG_ERROR("Failed to load: " + file.getFullPathName());
```

### Audio Processing Pattern

```cpp
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;

    // Clear output
    buffer.clear();

    // Process audio
    audioEngine.process(buffer, midiMessages);

    // Apply gain
    if (gain != 1.0f) {
        buffer.applyGain(gain);
    }
}
```

### Triggering Drums (Synthesis)

```cpp
BeatBox::DrumSynthParameters params;
params.pitch = 0.6f;      // 0.0 - 1.0
params.decay = 0.8f;      // Longer decay
params.tone = 0.5f;       // Mid tone
params.snap = 0.3f;       // Some attack
params.velocity = 1.0f;   // Full velocity
params.pan = 0.5f;        // Center

synthEngine.triggerDrum(BeatBox::DrumType::Kick, params);
```

### Loading Samples

```cpp
juce::File sampleFile("/path/to/sample.wav");

if (sampler.loadSample(padIndex, sampleFile)) {
    LOG_INFO("Sample loaded successfully");

    // Configure pad parameters
    auto& pad = sampler.getPad(padIndex);
    pad.pitch = -2.0f;    // -2 semitones
    pad.pan = 0.7f;       // Slightly right
    pad.gain = -3.0f;     // -3 dB
    sampler.updatePad(padIndex, pad);
} else {
    LOG_ERROR("Failed to load sample");
}
```

### Creating GUI Components

```cpp
// Create drum pad
auto pad = std::make_unique<BeatBox::DrumPadComponent>(padIndex);

// Set properties
pad->setPadName("Kick");
pad->setPadColour(juce::Colours::red);
pad->setHasSample(true);

// Set callbacks
pad->onPadTriggered = [this](int index, float velocity) {
    sampler.triggerPad(index, velocity);
};

pad->onPadRightClick = [this](int index) {
    showContextMenu(index);
};

// Add to UI
addAndMakeVisible(pad.get());
```

### Parameter Attachments

```cpp
// In PluginEditor constructor
masterGainAttachment = std::make_unique<
    juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(),
        "masterGain",
        masterGainSlider
);

// Slider automatically syncs with parameter
```

### Kit Save/Load

```cpp
// Save kit
auto kitXml = sampler.createKitXml();
if (kitXml.writeTo(file)) {
    LOG_INFO("Kit saved: " + file.getFullPathName());
}

// Load kit
std::unique_ptr<juce::XmlElement> xml(
    juce::XmlDocument::parse(file)
);
if (xml && sampler.loadKitFromXml(*xml)) {
    LOG_INFO("Kit loaded: " + file.getFullPathName());
}
```

### State Serialization

```cpp
// Save state
void getStateInformation(juce::MemoryBlock& destData) override {
    auto state = parameters.copyState();

    // Add custom data
    auto kitXml = audioEngine.getSampler().createKitXml();
    state.appendChild(juce::ValueTree::fromXml(kitXml), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

// Load state
void setStateInformation(const void* data, int sizeInBytes) override {
    std::unique_ptr<juce::XmlElement> xmlState(
        getXmlFromBinary(data, sizeInBytes)
    );

    if (xmlState) {
        auto state = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(state);

        // Load custom data
        if (auto* kitXml = xmlState->getChildByName("BeatBoxKit")) {
            audioEngine.getSampler().loadKitFromXml(*kitXml);
        }
    }
}
```

---

## DSP Synthesis Examples

### Kick Drum Synthesis

```cpp
void synthesizeKick(juce::AudioBuffer<float>& buffer, int start, int num) {
    float baseFreq = 50.0f + params.pitch * 100.0f;  // 50-150 Hz
    float pitchDecay = 0.05f + params.snap * 0.1f;

    for (int i = 0; i < num; ++i) {
        float env = getEnvelope();
        if (!active) break;

        // Exponential pitch sweep
        double envTime = envelopePhase / sampleRate;
        float currentFreq = baseFreq * (1.0f + 4.0f * std::exp(-envTime / pitchDecay));

        // Generate sine wave
        phaseIncrement = currentFreq / sampleRate;
        float sample = std::sin(phase * juce::MathConstants<double>::twoPi);
        phase += phaseIncrement;
        if (phase >= 1.0) phase -= 1.0;

        // Add click
        float click = params.snap * getNoiseValue() * std::exp(-envTime / 0.005f);

        // Combine and apply envelope
        sample = (sample * 0.7f + click * 0.3f) * env * params.velocity;

        // Apply panning
        buffer.addSample(0, start + i, sample * panLeft);
        buffer.addSample(1, start + i, sample * panRight);
    }
}
```

### Snare Drum Synthesis

```cpp
void synthesizeSnare(juce::AudioBuffer<float>& buffer, int start, int num) {
    float bodyFreq = 180.0f + params.pitch * 120.0f;

    // Setup filter for noise
    filter1.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    filter1.setCutoffFrequency(2000.0f + params.tone * 6000.0f);
    filter1.setResonance(0.5f);

    for (int i = 0; i < num; ++i) {
        float env = getEnvelope();
        if (!active) break;

        // Body tone (sine)
        phaseIncrement = bodyFreq / sampleRate;
        float body = std::sin(phase * juce::MathConstants<double>::twoPi) * 0.3f;
        phase += phaseIncrement;
        if (phase >= 1.0) phase -= 1.0;

        // Noise (filtered)
        float noise = getNoiseValue();
        noise = filter1.processSample(0, noise) * 0.7f;

        // Mix body and noise based on tone parameter
        float sample = (body * (1.0f - params.tone) + noise * params.tone)
                       * env * params.velocity;

        buffer.addSample(0, start + i, sample * panLeft);
        buffer.addSample(1, start + i, sample * panRight);
    }
}
```

### Hi-Hat Synthesis

```cpp
void synthesizeHiHat(juce::AudioBuffer<float>& buffer, int start, int num, bool open) {
    // High-pass filtered noise
    filter1.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    filter1.setCutoffFrequency(7000.0f + params.tone * 5000.0f);
    filter1.setResonance(0.7f);

    // Open hat has longer decay
    if (open) envelopeDecay *= 3.0;

    for (int i = 0; i < num; ++i) {
        float env = getEnvelope();
        if (!active) break;

        float noise = getNoiseValue();
        float filtered = filter1.processSample(0, noise);

        // Add metallic tone
        float metallic = std::sin(phase * 13.0) * std::sin(phase * 17.0) * 0.2f;
        phase += 0.001;

        float sample = (filtered * 0.8f + metallic * 0.2f) * env * params.velocity;

        buffer.addSample(0, start + i, sample * panLeft * 0.6f);
        buffer.addSample(1, start + i, sample * panRight * 0.6f);
    }
}
```

---

## Sample Playback with Pitch Shift

```cpp
void DrumVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                int startSample, int numSamples) {
    auto* playingSound = dynamic_cast<DrumSound*>(getCurrentlyPlayingSound().get());
    if (!playingSound) return;

    auto& data = playingSound->getAudioData();
    const int totalSamples = data.getNumSamples();

    while (--numSamples >= 0) {
        float envValue = adsr.getNextSample();

        if (!adsr.isActive()) {
            clearCurrentNote();
            break;
        }

        int currentPosition = reverse
            ? totalSamples - 1 - (int)sourceSamplePosition
            : (int)sourceSamplePosition;

        if (currentPosition < 0 || currentPosition >= totalSamples) {
            clearCurrentNote();
            break;
        }

        // Linear interpolation for pitch shifting
        const float alpha = (float)(sourceSamplePosition - (int)sourceSamplePosition);

        for (int channel = 0; channel < numChannels; ++channel) {
            float sample1 = data.getSample(channel, currentPosition);
            float sample2 = (currentPosition + 1 < totalSamples)
                ? data.getSample(channel, currentPosition + 1)
                : 0.0f;

            float interpolated = sample1 + alpha * (sample2 - sample1);
            interpolated *= envValue * level;

            // Apply panning
            if (channel == 0) {
                outputBuffer.addSample(0, startSample, interpolated * panLeft);
            } else {
                outputBuffer.addSample(1, startSample, interpolated * panRight);
            }
        }

        // Advance with pitch ratio
        sourceSamplePosition += pitchRatio;
        ++startSample;
    }
}
```

---

## MIDI Mapping

```cpp
// GM Drum Map (default pads)
const std::array<int, 16> midiNotes = {
    36,  // Kick (C1)
    38,  // Snare (D1)
    42,  // Closed Hat (F#1)
    46,  // Open Hat (A#1)
    39,  // Clap (D#1)
    50,  // High Tom (D2)
    48,  // Mid Tom (C2)
    45,  // Low Tom (A1)
    37,  // Rim (C#1)
    56,  // Cowbell (G#2)
    49,  // Crash (C#2)
    51,  // Ride (D#2)
    44,  // Pedal Hat (G#1)
    54,  // Tambourine (F#2)
    55,  // Splash (G2)
    57   // Crash 2 (A2)
};

// Handle MIDI in processBlock
for (const auto metadata : midiMessages) {
    auto message = metadata.getMessage();

    if (message.isNoteOn()) {
        int noteNumber = message.getNoteNumber();
        float velocity = message.getFloatVelocity();

        // Find pad with this MIDI note
        for (int i = 0; i < 16; ++i) {
            if (pads[i].midiNote == noteNumber) {
                sampler.triggerPad(i, velocity);
                break;
            }
        }
    }
}
```

---

## GUI Styling

```cpp
// Dark theme colors
const juce::Colour backgroundColor = juce::Colour(0xff1a1a1a);
const juce::Colour headerColor = juce::Colour(0xff2a2a2a);

// Gradient background
g.setGradientFill(juce::ColourGradient(
    topColor, 0.0f, 0.0f,
    bottomColor, 0.0f, (float)bounds.getHeight(),
    false
));

// Rounded rectangles
g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 2.0f);

// Equal-power panning
float panLeft = std::cos(pan * juce::MathConstants<float>::halfPi);
float panRight = std::sin(pan * juce::MathConstants<float>::halfPi);
```

---

## Debugging Tips

```bash
# Build in debug mode
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Run with debugger in CLion
# Set breakpoints in:
# - processBlock() for audio issues
# - loadSample() for file loading issues
# - triggerDrum() for synthesis issues

# Check assertions
# JUCE has many helpful jassert() statements

# Enable verbose logging
Logger::getInstance().setLogLevel(LogLevel::Debug);

# Monitor CPU in real-time
# Watch the CPU load label in the GUI
```

---

## Common Modifications

### Add a New Drum Type

1. Add to enum in `DrumSynthEngine.h`:
```cpp
enum class DrumType {
    // ... existing types ...
    Ride,  // NEW
};
```

2. Add synthesis method:
```cpp
void synthesizeRide(juce::AudioBuffer<float>& buffer, int start, int num);
```

3. Add case to process():
```cpp
case DrumType::Ride:
    synthesizeRide(outputBuffer, startSample, numSamples);
    break;
```

### Add Effect to Audio Engine

```cpp
// In AudioEngine.h
juce::dsp::Reverb reverb;

// In prepare()
juce::dsp::ProcessSpec spec{sampleRate, samplesPerBlock, numChannels};
reverb.prepare(spec);

// In process()
juce::dsp::AudioBlock<float> block(buffer);
juce::dsp::ProcessContextReplacing<float> context(block);
reverb.process(context);
```

### Add Parameter

```cpp
// In createParameterLayout()
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    "reverbMix",
    "Reverb Mix",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
    0.3f  // default
));

// In header
std::atomic<float>* reverbMixParam = nullptr;

// In constructor
reverbMixParam = parameters.getRawParameterValue("reverbMix");

// In processBlock()
float mix = reverbMixParam->load();
reverb.setWetLevel(mix);
```

---

## .gitignore

```
# CLion / IntelliJ
.idea/
cmake-build-*/
.DS_Store

# Build directories
build/
Builds/

# JUCE generated
JuceLibraryCode/
*_artefacts/

# Xcode
*.xcodeproj
xcuserdata/
DerivedData/

# Visual Studio
.vs/
*.vcxproj
x64/
Debug/
Release/

# Compiled binaries
*.vst3
*.component
*.app

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake

# Logs
*.log
```

---

This document provides all the essential code patterns and commands to replicate and extend the BC BeatBox project!
