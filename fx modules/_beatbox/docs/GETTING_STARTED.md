# Getting Started with BC BeatBox

## Build Status

✅ **Successfully Built!**

The BC BeatBox drum machine has been successfully compiled in all formats:
- ✅ VST3 Plugin
- ✅ AU (Audio Unit) Plugin
- ✅ Standalone Application

## Built Artifacts

Your plugins are located at:

```
build/BeatBox_artefacts/Release/
├── VST3/
│   └── BC BeatBox.vst3
├── AU/
│   └── BC BeatBox.component
└── Standalone/
    └── BC BeatBox.app
```

## Installation

### macOS Installation

#### VST3 Plugin
```bash
cp -r "build/BeatBox_artefacts/Release/VST3/BC BeatBox.vst3" \
      ~/Library/Audio/Plug-Ins/VST3/
```

#### AU Plugin
```bash
cp -r "build/BeatBox_artefacts/Release/AU/BC BeatBox.component" \
      ~/Library/Audio/Plug-Ins/Components/
```

After copying, run:
```bash
killall -9 AudioComponentRegistrar
```

#### Standalone App
```bash
cp -r "build/BeatBox_artefacts/Release/Standalone/BC BeatBox.app" \
      /Applications/
```

## Quick Start Guide

### 1. Launch the Plugin

**In a DAW:**
1. Scan for new plugins
2. Load "BC BeatBox" as an instrument
3. Create a MIDI track or route MIDI to it

**Standalone:**
1. Open `/Applications/BC BeatBox.app`
2. Configure audio/MIDI settings in preferences

### 2. First Sounds

#### Using Synthesis Mode:
1. Set **Engine Mode** to "Synthesis"
2. Click any drum pad to trigger synthesized drums
3. Adjust **Master Gain** to taste

#### Using Samples:
1. Set **Engine Mode** to "Sampling"
2. **Right-click** a pad → "Load Sample..."
3. Browse and select a drum sample (WAV, AIFF, etc.)
4. Click the pad to trigger your sample

#### Using Hybrid Mode:
1. Set **Engine Mode** to "Hybrid"
2. Use **Synth/Sample Mix** slider to blend both
3. 0% = All synthesis, 100% = All samples

### 3. Playing with MIDI

The pads are mapped to MIDI notes (GM drum map):

| Pad  | MIDI Note | Note Name |
|------|-----------|-----------|
| 1    | C1 (36)   | Kick      |
| 2    | D1 (38)   | Snare     |
| 3    | F#1 (42)  | Closed Hat|
| 4    | A#1 (46)  | Open Hat  |
| ... and so on |

### 4. Pad Controls

**Left Click**: Trigger pad (velocity depends on distance from center)

**Right Click Menu**:
- Load Sample...
- Clear Sample
- Mute
- Solo

### 5. Saving Your Kit

1. Configure your pads with samples
2. Click **Save Kit** button
3. Choose a location (default: `~/Documents/BeatBox/Kits/`)
4. Name your kit (`.xml` file)

### 6. Loading Kits

1. Click **Load Kit** button
2. Browse to your saved kit file
3. All pads will load with their samples

## Development Workflow

### Rebuilding After Changes

```bash
cd /Users/sonic.design/CLionProjects/_BC_BeatBox
cmake --build build --config Release -j8
```

### Clean Build

```bash
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j8
```

### Debug Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j8
```

Debug builds include:
- Debug symbols
- JUCE assertions enabled
- More verbose logging

### Opening in CLion

1. Open CLion
2. File → Open → Select project folder
3. CLion will auto-detect CMake configuration
4. Select "BeatBox" target from dropdown
5. Build with ⌘F9 or Run/Debug

## Project Architecture

```
BC BeatBox Architecture:

PluginProcessor
    ↓
AudioEngine ←→ Parameters
    ├── DrumSynthEngine (16 voices)
    │   └── Synthesis algorithms
    └── DrumSampler (32 voices)
        └── Sample playback

PluginEditor
    ├── DrumPadComponent (x16)
    ├── SequencerComponent
    └── Controls
```

## Key Features Implemented

### ✅ Audio Engine
- Dual-engine architecture (Synth + Sample)
- 16-voice polyphonic synthesis
- 32-voice sample playback
- Real-time switching between modes
- CPU usage monitoring

### ✅ Synthesis Engine
- Kick: Pitch sweep + sine + click
- Snare: Body tone + filtered noise
- Hi-hats: Filtered noise (open/closed)
- Clap: Multi-burst noise
- Toms: Resonant pitch decay
- Rim: Short bandpass click
- Cowbell: Inharmonic dual oscillator

### ✅ Sample Engine
- Multi-format support (WAV, AIFF, MP3, OGG, FLAC)
- Per-pad parameters (pitch, pan, gain, ADSR)
- Reverse playback
- Solo/Mute per pad
- Kit save/load system

### ✅ GUI
- 16-pad grid (4x4)
- Visual feedback and animations
- Right-click context menus
- Parameter controls
- CPU load display

### ✅ Professional Features
- Thread-safe logging system
- Parameter automation (DAW)
- State save/restore
- Cross-platform (macOS/Windows)

## Common Issues & Solutions

### Plugin Not Showing in DAW

**Solution:**
1. Check plugin was copied to correct location
2. Rescan plugins in DAW
3. On macOS: `killall -9 AudioComponentRegistrar`
4. Restart DAW

### Build Errors

**"JuceHeader.h not found"**
- Already fixed! We use module-specific includes

**CMake errors**
- Ensure CMake 3.22+: `brew upgrade cmake`
- Delete `build/` and reconfigure

### No Sound

1. Check **Master Gain** isn't at minimum
2. Verify correct **Engine Mode** selected
3. In Sampling mode, ensure samples are loaded
4. Check MIDI is routed correctly
5. Verify audio output routing in DAW

### Samples Not Loading

1. Check file format is supported
2. Verify file permissions
3. Check log file: `~/Library/Logs/BeatBox/beatbox.log`
4. Try WAV format (most reliable)

## Log Files

Logs are written to:
```
macOS: ~/Library/Logs/BeatBox/beatbox.log
```

Useful for debugging:
```bash
tail -f ~/Library/Logs/BeatBox/beatbox.log
```

## Performance Tips

1. **Buffer Size**: 256-512 samples for best balance
2. **Sample Rate**: 44.1 or 48 kHz recommended
3. **Voice Count**: Monitor CPU load display
4. **Hybrid Mode**: May use more CPU than single mode

## Next Steps

1. **Add Your Samples**: Build a custom kit
2. **Experiment**: Try Hybrid mode with different mixes
3. **Automate**: Map parameters to your DAW's automation
4. **MIDI Map**: Assign to your drum controller

## Future Development

Planned features:
- Step sequencer (UI is placeholder)
- Built-in effects (reverb, delay, distortion)
- Preset browser
- Waveform display for samples
- Per-pad effects sends

## Need Help?

- **Documentation**: See `docs/ARCHITECTURE.md` and `docs/API.md`
- **Issues**: Check log files first
- **Source Code**: Well-documented C++ with Doxygen comments

## Version Info

- **Version**: 1.0.0
- **Build Date**: 2025
- **JUCE Version**: 7.x
- **C++ Standard**: C++20
- **Compiler**: Apple Clang 17.0 / MSVC 19.x

---

**Congratulations! Your drum machine is ready to use! 🥁**
