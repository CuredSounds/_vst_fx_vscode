# BC BeatBox - Professional Drum Machine VST/AU Plugin

A powerful hybrid drum machine plugin combining synthesis and sampling, built with JUCE framework.

## Features

### 🎵 Dual Engine Architecture
- **Synthesis Engine**: Classic drum synthesis with 10 drum types
  - Kick, Snare, Hi-hats (Open/Closed), Clap
  - Toms (High/Mid/Low), Rim, Cowbell
  - Real-time parameter control (Pitch, Decay, Tone, Snap)

- **Sample Engine**: Professional 16-pad sampler
  - Support for WAV, AIFF, MP3, OGG, FLAC formats
  - Per-pad ADSR envelope control
  - Pitch shifting, panning, gain control
  - Reverse playback capability
  - Solo/Mute functionality

- **Hybrid Mode**: Layer synthesis and samples

### 🎹 16-Pad Grid Interface
- Visual feedback with customizable colors
- Velocity-sensitive triggering
- MIDI mappable (GM drum map compatible)
- Right-click context menus for quick access
- Load/clear samples per pad

### 💾 Kit Management
- Save and load complete drum kits
- XML-based kit format (human-readable)
- Kit directory management

### 🔧 Technical Features
- VST3 and AU plugin formats
- Standalone application included
- Low latency performance
- CPU usage monitoring
- Thread-safe logging system
- Professional error handling

## System Requirements

- macOS 10.13+ (Intel/Apple Silicon)
- Windows 10+ (64-bit)
- VST3 or AU compatible DAW
- 4GB RAM minimum

## Building from Source

### Prerequisites

1. **CMake** (3.22 or higher)
   ```bash
   brew install cmake
   ```

2. **Xcode** (macOS) or **Visual Studio** (Windows)
   - Xcode: Install from App Store
   - Command Line Tools: `xcode-select --install`

3. **JUCE Framework** (already included in this repo)

### Build Steps

1. Clone and prepare:
   ```bash
   cd /Users/sonic.design/CLionProjects/_BC_BeatBox
   mkdir build
   cd build
   ```

2. Configure with CMake:
   ```bash
   cmake ..
   ```

3. Build:
   ```bash
   cmake --build . --config Release
   ```

4. Plugins will be in:
   - VST3: `build/BeatBox_artefacts/Release/VST3/`
   - AU: `build/BeatBox_artefacts/Release/AU/`
   - Standalone: `build/BeatBox_artefacts/Release/Standalone/`

### Building in CLion

1. Open project in CLion
2. Select "BeatBox" target
3. Build > Build Project (⌘F9)
4. Plugins are automatically copied to system plugin folders

## Project Structure

```
_BC_BeatBox/
├── Source/
│   ├── Audio/
│   │   ├── AudioEngine.cpp/h          # Main audio engine
│   │   ├── DrumSynthEngine.cpp/h      # Synthesis engine
│   │   ├── DrumSampler.cpp/h          # Sample player
│   │   └── DrumVoice.cpp/h            # Voice management
│   ├── GUI/
│   │   ├── DrumPadComponent.cpp/h     # Drum pad UI
│   │   └── SequencerComponent.cpp/h   # Sequencer (future)
│   ├── Utils/
│   │   └── Logger.cpp/h               # Logging system
│   ├── PluginProcessor.cpp/h          # Main processor
│   └── PluginEditor.cpp/h             # Main GUI
├── Resources/                         # Assets, icons
├── Samples/                           # Demo drum samples
├── Tests/                             # Unit tests (future)
├── docs/                              # Documentation
├── JUCE/                              # JUCE framework
├── CMakeLists.txt                     # Build configuration
└── README.md                          # This file
```

## Usage

### Loading Samples
1. Right-click any pad
2. Select "Load Sample..."
3. Choose an audio file
4. Pad updates with sample name and color

### Playing Drums
- **Click pads**: Trigger sounds
- **MIDI controller**: Sends MIDI notes (C1-D#2 = pads 1-16)
- **Computer keyboard**: Use MIDI keyboard in DAW

### Kit Management
1. **Save Kit**: Saves current pad configuration and sample references
2. **Load Kit**: Restores previously saved kit

### Engine Modes
- **Synthesis**: Use only synthesized drums
- **Sampling**: Use only loaded samples
- **Hybrid**: Layer both (adjustable mix)

## Logging

Logs are automatically written to:
- **macOS**: `~/Library/Logs/BeatBox/beatbox.log`
- **Windows**: `%APPDATA%/BeatBox/Logs/beatbox.log`

Log levels: DEBUG, INFO, WARNING, ERROR, CRITICAL

## Parameters (Automatable)

- **Master Gain**: -60dB to +12dB
- **Engine Mode**: Synthesis / Sampling / Hybrid
- **Synth/Sample Mix**: 0.0 (all synth) to 1.0 (all samples)

## Known Issues

- Sequencer UI is placeholder (coming in v2.0)
- Sample streaming for large files not yet implemented
- No built-in effects (reverb/delay) yet

## Roadmap

### Version 1.1
- [ ] Built-in effects (reverb, delay, distortion)
- [ ] Per-pad effects sends
- [ ] Preset browser

### Version 2.0
- [ ] 16-step sequencer
- [ ] Pattern chaining
- [ ] MIDI export

### Version 2.1
- [ ] Sample editor with waveform view
- [ ] Multi-layer support per pad
- [ ] Round-robin sample support

## Development

### Code Style
- Modern C++20
- JUCE coding standards
- Doxygen documentation comments
- Namespace: `BeatBox`

### Testing
```bash
cd build
ctest
```

### Debugging
- Enable debug logging: Set log level to DEBUG in Logger
- Use JUCE's Projucer for debugging
- Check log files for error traces

## License

Copyright © 2025 Broadcast Concepts
All rights reserved.

## Credits

- **JUCE Framework**: [juce.com](https://juce.com)
- **Development**: Broadcast Concepts
- **DSP Algorithms**: Based on classic analog drum synthesis

## Support

- Issues: [GitHub Issues](https://github.com/broadcastconcepts/beatbox/issues)
- Email: info@broadcastconcepts.com
- Documentation: [docs.broadcastconcepts.com/beatbox](https://docs.broadcastconcepts.com/beatbox)

---

**Built with ❤️ using JUCE and CLion on Apple Silicon**
