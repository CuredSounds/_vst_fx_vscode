# BeatBox API Reference

## Public API Classes

### AudioEngine

Main audio processing engine combining synthesis and sampling.

```cpp
class AudioEngine {
public:
    // Initialization
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);

    // Processing
    void process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    // Engine Configuration
    void setEngineMode(EngineMode mode);
    EngineMode getEngineMode() const;

    // Parameters
    void setMasterGain(float gainDb);
    void setSynthSampleMix(float mix);  // 0.0 = synth, 1.0 = samples

    // Access to sub-engines
    DrumSynthEngine& getSynthEngine();
    DrumSampler& getSampler();

    // Utility
    void reset();
    float getCpuLoad() const;
};
```

**Example Usage**:
```cpp
AudioEngine engine;
engine.prepare(44100.0, 512, 2);
engine.setEngineMode(EngineMode::Hybrid);
engine.setMasterGain(0.0f);  // 0 dB
engine.setSynthSampleMix(0.5f);  // 50/50 mix

// In audio callback:
engine.process(audioBuffer, midiMessages);
```

---

### DrumSynthEngine

Polyphonic synthesis engine for electronic drums.

```cpp
class DrumSynthEngine {
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void triggerDrum(DrumType type, const DrumSynthParameters& params);
    void process(juce::AudioBuffer<float>& buffer);
    void stopAll();
    int getNumActiveVoices() const;
    void setGain(float gainDb);
};

enum class DrumType {
    Kick, Snare, ClosedHat, OpenHat, Clap,
    HighTom, MidTom, LowTom, Rim, Cowbell
};

struct DrumSynthParameters {
    float pitch = 0.5f;     // 0.0 - 1.0
    float decay = 0.5f;     // 0.0 - 1.0
    float tone = 0.5f;      // 0.0 - 1.0
    float snap = 0.5f;      // 0.0 - 1.0
    float velocity = 1.0f;  // 0.0 - 1.0
    float pan = 0.5f;       // 0.0 (L) - 1.0 (R)
    float tuning = 0.0f;    // -12 to +12 semitones
};
```

**Example Usage**:
```cpp
DrumSynthEngine synth;
synth.prepare(44100.0, 512);

DrumSynthParameters params;
params.pitch = 0.6f;     // Higher pitch
params.decay = 0.8f;     // Longer decay
params.velocity = 1.0f;  // Full velocity

synth.triggerDrum(DrumType::Kick, params);
```

---

### DrumSampler

Sample-based drum machine with 16 pads.

```cpp
class DrumSampler {
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    // Sample Management
    bool loadSample(int padIndex, const juce::File& sampleFile);
    void clearPad(int padIndex);
    void triggerPad(int padIndex, float velocity = 1.0f);

    // Pad Access
    DrumPad& getPad(int padIndex);
    const DrumPad& getPad(int padIndex) const;
    void updatePad(int padIndex, const DrumPad& pad);

    // Utility
    static constexpr int getNumPads() { return 16; }
    bool isAnySoloed() const;

    // Kit Management
    juce::XmlElement createKitXml() const;
    bool loadKitFromXml(const juce::XmlElement& xml);
};

struct DrumPad {
    juce::String name;
    juce::File sampleFile;
    int midiNote;
    float pitch;     // Semitones
    float pan;       // 0.0 (L) - 1.0 (R)
    float gain;      // dB
    bool reverse;
    bool muted;
    bool solo;
    juce::ADSR::Parameters adsr;
    juce::Colour colour;
};
```

**Example Usage**:
```cpp
DrumSampler sampler;
sampler.prepare(44100.0, 512);

// Load a kick sample
juce::File kickFile("/path/to/kick.wav");
sampler.loadSample(0, kickFile);

// Configure the pad
DrumPad& pad = sampler.getPad(0);
pad.gain = -3.0f;  // -3 dB
pad.pan = 0.5f;    // Center
pad.pitch = -2.0f; // -2 semitones
sampler.updatePad(0, pad);

// Trigger the pad
sampler.triggerPad(0, 0.8f);  // 80% velocity
```

---

### Logger

Thread-safe logging utility.

```cpp
class Logger {
public:
    static Logger& getInstance();

    void initialize(const juce::File& logFilePath = juce::File());
    void log(LogLevel level, const juce::String& message,
             const char* file = nullptr, int line = 0);

    void setLogLevel(LogLevel level);
    void setConsoleOutput(bool enabled);
    void flush();

    juce::File getLogFilePath() const;
};

enum class LogLevel {
    Debug, Info, Warning, Error, Critical
};
```

**Macros**:
```cpp
LOG_DEBUG("Debug message");
LOG_INFO("Info message");
LOG_WARNING("Warning message");
LOG_ERROR("Error message");
LOG_CRITICAL("Critical message");
```

**Example Usage**:
```cpp
auto& logger = Logger::getInstance();
logger.initialize();  // Use default location
logger.setLogLevel(LogLevel::Info);

LOG_INFO("Application started");
LOG_WARNING("Sample rate is low: " + String(sampleRate));

try {
    // ... code ...
} catch (const std::exception& e) {
    LOG_ERROR(String("Exception: ") + e.what());
}
```

---

## GUI Components

### DrumPadComponent

Visual drum pad with interaction.

```cpp
class DrumPadComponent : public juce::Component {
public:
    DrumPadComponent(int padIndex);

    // Configuration
    void setPadName(const juce::String& name);
    void setPadColour(juce::Colour colour);
    void setHasSample(bool hasSample);
    void setMuted(bool muted);
    void setSoloed(bool soloed);

    // Animation
    void triggerAnimation();

    // Callbacks
    std::function<void(int padIndex, float velocity)> onPadTriggered;
    std::function<void(int padIndex)> onPadRightClick;
};
```

**Example Usage**:
```cpp
auto pad = std::make_unique<DrumPadComponent>(0);

pad->setPadName("Kick");
pad->setPadColour(Colours::red);
pad->setHasSample(true);

pad->onPadTriggered = [](int index, float velocity) {
    // Handle pad trigger
    sampler.triggerPad(index, velocity);
};

pad->onPadRightClick = [](int index) {
    // Show context menu
    showPadMenu(index);
};

addAndMakeVisible(pad.get());
```

---

## MIDI Mapping

### Default MIDI Note Map (GM Drums)

| Pad | MIDI Note | Note Name | Default Sound |
|-----|-----------|-----------|---------------|
| 0   | 36        | C1        | Kick          |
| 1   | 38        | D1        | Snare         |
| 2   | 42        | F#1       | Closed Hi-hat |
| 3   | 46        | A#1       | Open Hi-hat   |
| 4   | 39        | D#1       | Clap          |
| 5   | 50        | D2        | High Tom      |
| 6   | 48        | C2        | Mid Tom       |
| 7   | 45        | A1        | Low Tom       |
| 8   | 37        | C#1       | Rim           |
| 9   | 56        | G#2       | Cowbell       |
| 10  | 49        | C#2       | Crash         |
| 11  | 51        | D#2       | Ride          |
| 12  | 44        | G#1       | Pedal Hi-hat  |
| 13  | 54        | F#2       | Tambourine    |
| 14  | 55        | G2        | Splash        |
| 15  | 57        | A2        | Crash 2       |

---

## Parameter IDs

For automation in DAWs:

| Parameter       | ID                | Range       | Default |
|-----------------|-------------------|-------------|---------|
| Master Gain     | `masterGain`      | -60 to +12  | 0.0 dB  |
| Engine Mode     | `engineMode`      | 0-2 (enum)  | 2       |
| Synth/Sample Mix| `synthSampleMix`  | 0.0 to 1.0  | 0.5     |

**Engine Mode Values**:
- 0 = Synthesis
- 1 = Sampling
- 2 = Hybrid

---

## File Formats

### Kit File Format (.xml)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<BeatBoxKit version="1.0">
  <Pad index="0"
       name="Kick"
       midiNote="36"
       pitch="0.0"
       pan="0.5"
       gain="0.0"
       reverse="false"
       muted="false"
       solo="false"
       sampleFile="/path/to/kick.wav">
    <ADSR attack="0.001"
          decay="0.0"
          sustain="1.0"
          release="0.05"/>
  </Pad>
  <!-- ... 15 more pads ... -->
</BeatBoxKit>
```

---

## Performance Guidelines

### Real-time Safety

**DO**:
- ✅ Use lock-free operations
- ✅ Pre-allocate buffers
- ✅ Use stack allocation
- ✅ Avoid branches in hot paths

**DON'T**:
- ❌ No `new`/`delete` in audio thread
- ❌ No file I/O
- ❌ No system calls
- ❌ No waiting/blocking
- ❌ No exceptions

### Buffer Sizes

Recommended: 128-512 samples
- Smaller = lower latency, higher CPU
- Larger = higher latency, lower CPU

### Voice Count

- Synthesis: 16 voices (fixed)
- Sampling: 32 voices (configurable)

### Sample Rate Support

- Minimum: 44.1 kHz
- Maximum: 192 kHz
- Recommended: 44.1 or 48 kHz

---

## Error Codes

### Return Values

- `true` / `false` for success/failure
- Check logs for detailed error messages

### Common Errors

| Error | Cause | Solution |
|-------|-------|----------|
| "Invalid pad index" | Index out of range | Use 0-15 |
| "File not found" | Invalid path | Check file exists |
| "Could not read audio file" | Unsupported format | Use WAV/AIFF |
| "No occurrences found" | Invalid MIDI note | Check MIDI map |

---

## Thread Model

### Audio Thread
- `processBlock()`
- Real-time priority
- No allocations
- Lock-free preferred

### Message Thread
- GUI updates
- Parameter changes
- File loading
- Logging

### Background Thread
- Kit loading/saving
- Sample scanning

---

## Memory Usage

### Typical Memory Footprint

- Plugin code: ~2 MB
- Per sample: ~file size × channels
- Working buffers: ~100 KB
- Total (no samples): ~5 MB
- Total (16 samples): ~20-50 MB

---

## Best Practices

### Loading Samples

```cpp
// GOOD: Async loading
juce::MessageManager::callAsync([this, file]() {
    sampler.loadSample(0, file);
});

// BAD: Blocking in audio thread
sampler.loadSample(0, file);  // DON'T DO THIS IN processBlock()!
```

### Parameter Updates

```cpp
// GOOD: Atomic parameters
std::atomic<float>* gainParam;
float gain = gainParam->load();

// GOOD: Double buffering
updateParametersFromUI();  // Message thread
applyParameters();         // Audio thread
```

### Error Handling

```cpp
// GOOD: Check and log
if (!sampler.loadSample(index, file)) {
    LOG_ERROR("Failed to load sample");
    return false;
}

// GOOD: Graceful degradation
if (sample == nullptr) {
    playbackMode = SynthesisOnly;
    LOG_WARNING("Switched to synthesis mode");
}
```

---

## Version Compatibility

### API Stability

- **v1.x**: Stable API
- **v2.x**: May add features, backward compatible
- **v3.x**: May break API

### Kit File Compatibility

- v1.0 kits will load in all future versions
- Newer features may not load in older versions
