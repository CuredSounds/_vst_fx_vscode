# BeatBox Architecture Documentation

## Overview

BC BeatBox is built with a modular, professional architecture designed for maintainability, performance, and extensibility.

## Architecture Layers

```
┌─────────────────────────────────────────┐
│         Plugin Host (DAW)               │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│      PluginProcessor (JUCE)             │
│  - Parameter management                 │
│  - State save/load                      │
│  - MIDI handling                        │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│          AudioEngine                    │
│  - Engine mode switching                │
│  - Buffer management                    │
│  - CPU monitoring                       │
└───────┬─────────────────────┬───────────┘
        │                     │
┌───────▼──────────┐  ┌──────▼────────────┐
│  DrumSynthEngine │  │   DrumSampler     │
│  - Voice pool    │  │   - JUCE Synth    │
│  - DSP synthesis │  │   - Sample mgmt   │
└──────────────────┘  └───────────────────┘
```

## Core Components

### 1. PluginProcessor
**File**: `Source/PluginProcessor.cpp/h`

**Responsibilities**:
- JUCE AudioProcessor interface
- Parameter tree management
- State serialization (save/load)
- MIDI message routing
- Audio buffer lifecycle

**Key Methods**:
- `prepareToPlay()`: Initialize audio engine
- `processBlock()`: Main audio callback
- `getStateInformation()`: Serialize state
- `setStateInformation()`: Deserialize state

### 2. AudioEngine
**File**: `Source/Audio/AudioEngine.cpp/h`

**Responsibilities**:
- Coordinate synthesis and sampling engines
- Engine mode management (Synth/Sample/Hybrid)
- Buffer mixing and gain control
- CPU load estimation

**Design Pattern**: Facade pattern - provides unified interface to complex subsystems

### 3. DrumSynthEngine
**File**: `Source/Audio/DrumSynthEngine.cpp/h`

**Responsibilities**:
- Polyphonic voice management (16 voices)
- DSP-based drum synthesis
- Per-drum-type synthesis algorithms

**Synthesis Techniques**:
- **Kick**: Exponential pitch sweep + sine + noise
- **Snare**: Body tone (sine) + filtered noise
- **Hi-hat**: High-pass filtered noise + metallic overtones
- **Clap**: Multi-burst filtered noise
- **Toms**: Resonant pitch decay
- **Rim**: Short bandpass filtered burst
- **Cowbell**: Dual square wave inharmonic synthesis

**Voice Management**:
- Voice stealing (oldest first)
- Envelope-based deactivation
- Thread-safe voice allocation

### 4. DrumSampler
**File**: `Source/Audio/DrumSampler.cpp/h`

**Responsibilities**:
- Sample loading and management
- JUCE Synthesiser integration
- 16-pad configuration
- Solo/Mute logic
- Kit serialization

**Design Pattern**: Adapter pattern - wraps JUCE Synthesiser for drum-specific use

### 5. DrumVoice
**File**: `Source/Audio/DrumVoice.cpp/h`

**Responsibilities**:
- JUCE SynthesiserVoice implementation
- Sample playback with interpolation
- ADSR envelope application
- Pitch shifting
- Pan control

**Features**:
- Linear interpolation for pitch shift
- Equal-power panning
- Reverse playback support

### 6. Logger
**File**: `Source/Utils/Logger.cpp/h`

**Responsibilities**:
- Thread-safe logging
- File and console output
- Log level filtering
- Automatic session management

**Design Pattern**: Singleton pattern

**Log Levels**:
1. DEBUG: Development info
2. INFO: General information
3. WARNING: Potential issues
4. ERROR: Recoverable errors
5. CRITICAL: Fatal errors

## GUI Architecture

### PluginEditor
**File**: `Source/PluginEditor.cpp/h`

**Responsibilities**:
- Main UI layout
- Component management
- Parameter bindings
- Timer-based UI updates (30 FPS)

### DrumPadComponent
**File**: `Source/GUI/DrumPadComponent.cpp/h`

**Responsibilities**:
- Visual drum pad representation
- Mouse interaction handling
- Animation management
- State visualization (mute/solo/loaded)

**Design Features**:
- Velocity-sensitive clicking (distance from center)
- Right-click context menu
- Visual feedback animation

## Data Flow

### Audio Processing Flow

```
MIDI In → PluginProcessor::processBlock()
           │
           ├─→ AudioEngine::process()
           │    │
           │    ├─→ DrumSynthEngine::process()
           │    │    └─→ DrumSynthVoice::process() [x16]
           │    │
           │    ├─→ DrumSampler::process()
           │    │    └─→ JUCE Synthesiser
           │    │         └─→ DrumVoice::renderNextBlock() [x32]
           │    │
           │    └─→ Mix buffers (if Hybrid mode)
           │
           └─→ Audio Out
```

### GUI Update Flow

```
Timer (30 FPS) → PluginEditor::timerCallback()
                   │
                   ├─→ Update CPU load display
                   └─→ updatePadUI()
                        └─→ For each DrumPadComponent:
                             - Update name
                             - Update colour
                             - Update sample status
                             - Update mute/solo state
```

### Parameter Flow

```
User Interaction → GUI Component → JUCE Parameter Attachment
                                     │
                                     ├─→ AudioProcessorValueTreeState
                                     │
                                     └─→ PluginProcessor::processBlock()
                                          └─→ AudioEngine (apply parameters)
```

## Thread Safety

### Audio Thread
- `processBlock()` - Real-time safe
- No allocations
- No file I/O
- Lock-free where possible

### Message Thread
- GUI updates
- File loading
- Parameter changes
- Logger writes

### Critical Sections
- `AudioEngine`: Voice access locked
- `Logger`: Mutex-protected writes

## Memory Management

### RAII Principles
- Smart pointers (`unique_ptr`, `shared_ptr`)
- JUCE reference counting
- Automatic resource cleanup

### Buffer Management
- Pre-allocated working buffers
- Reuse between process calls
- No dynamic allocation in audio thread

## File Formats

### Kit Files (.xml)
```xml
<BeatBoxKit version="1.0">
  <Pad index="0" name="Kick" midiNote="36" pitch="0.0" ...>
    <ADSR attack="0.001" decay="0.0" sustain="1.0" release="0.05"/>
  </Pad>
  ...
</BeatBoxKit>
```

### State Serialization
- JUCE ValueTree for parameters
- XML for sample references
- Binary for plugin state

## Performance Considerations

### CPU Optimization
- Voice deactivation when envelope < 0.001
- Buffer size optimization
- Filter state caching
- Smooth CPU load estimation

### Memory Optimization
- Sample references (not copies)
- Shared filter objects
- Stack allocation where possible

## Extension Points

### Adding New Drum Types
1. Add enum to `DrumType`
2. Implement synthesis method in `DrumSynthVoice`
3. Add case to `process()` switch

### Adding Effects
1. Create effect class in `Source/Audio/Effects/`
2. Add to `AudioEngine`
3. Add GUI controls in `PluginEditor`

### Adding Sequencer
1. Implement `SequencerEngine` class
2. Add to `AudioEngine`
3. Complete `SequencerComponent` GUI

## Error Handling

### Strategy
- Defensive checks at public APIs
- Logging for all errors
- Graceful degradation
- User feedback via GUI

### Example
```cpp
bool DrumSampler::loadSample(int padIndex, const juce::File& file) {
    if (padIndex < 0 || padIndex >= numPads) {
        LOG_ERROR("Invalid pad index");
        return false;
    }

    if (!file.existsAsFile()) {
        LOG_ERROR("File not found: " + file.getFullPathName());
        return false;
    }

    // ... load sample ...

    LOG_INFO("Sample loaded successfully");
    return true;
}
```

## Testing Strategy

### Unit Tests (Planned)
- Synthesis algorithm correctness
- Sample playback accuracy
- Parameter range validation
- State serialization

### Integration Tests
- DAW compatibility
- Format validation (VST3/AU)
- Performance benchmarks

## Build System

### CMake Configuration
- JUCE integration via `add_subdirectory()`
- Target-based configuration
- Automatic plugin copying (macOS)
- Multi-format build (VST3, AU, Standalone)

### Dependencies
- JUCE 7.x (included)
- C++20 standard library
- Platform SDKs (macOS/Windows)

## Future Architecture Improvements

### Version 2.0
- [ ] Separate DSP thread
- [ ] Lock-free audio queue
- [ ] Multi-layer samples per pad
- [ ] Effect chain architecture
- [ ] Preset management system

### Version 3.0
- [ ] Scripting support (Lua/JavaScript)
- [ ] Plugin-in-plugin architecture
- [ ] Distributed processing support
