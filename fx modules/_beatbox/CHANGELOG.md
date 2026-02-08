# BeatBox - Changelog

## Version 1.0.1 (2025-11-22) - Crash Fix

### Fixed
- **Critical**: Fixed crash when opening plugin in Nuendo/Cubase
  - Issue: `resized()` was called before drum pads were initialized
  - Solution: Added null pointer check in `PluginEditor::resized()`
  - Location: `Source/PluginEditor.cpp:240`

### Changed
- Updated vendor/company name from "BroadcastConcepts" to "Sonicdesign"
- Updated plugin display name from "BC BeatBox" to "BeatBox"
- Updated manufacturer code from "BrCo" to "SncD"
- Updated bundle ID to `com.sonicdesign.beatbox`

---

## Version 1.0.0 (2025-11-22) - Initial Release

### Features
- Dual-engine drum machine (Synthesis + Sampling)
- 10 synthesized drum types with DSP algorithms
- 16-pad sample playback with multi-format support
- Hybrid mode to layer synthesis and samples
- 16 interactive drum pads with visual feedback
- Solo/Mute per pad
- Kit save/load system (XML format)
- Thread-safe logging system
- MIDI input with GM drum map
- Real-time parameter automation
- CPU usage monitoring

### Supported Formats
- VST3 (macOS)
- AU (Audio Units)
- Standalone application

### Known Issues
- Compiler warnings (sign conversion, deprecated Font constructor)
- Sequencer UI is placeholder only
- No built-in effects yet

---

## Release Notes

### Installation
```bash
# VST3
~/Library/Audio/Plug-Ins/VST3/BeatBox.vst3

# AU
~/Library/Audio/Plug-Ins/Components/BeatBox.component
```

### Finding in DAW
- **Manufacturer**: Sonicdesign
- **Plugin Name**: BeatBox
- **Category**: Instrument / Synth

### System Requirements
- macOS 10.13+
- Apple Silicon (M-series) or Intel Mac
- VST3 or AU compatible DAW
- 4GB RAM minimum

---

## Previous Builds

### Debug Build Issues
- Initial debug builds had code signing issues
- Standalone app was incomplete in debug mode
- Caused crashes in some DAWs (Nuendo, Cubase)

**Solution**: Always use Release builds for production

---

## Upgrade Notes

### From BroadcastConcepts Build
If you had the old "BC BeatBox" plugin installed:

1. Remove old plugins:
```bash
rm -rf ~/Library/Audio/Plug-Ins/VST3/BC\ BeatBox.vst3
rm -rf ~/Library/Audio/Plug-Ins/Components/BC\ BeatBox.component
```

2. Clear cache:
```bash
killall -9 AudioComponentRegistrar
```

3. Rescan plugins in your DAW

4. Projects with old plugin will need to be updated

---

## Build Information

### v1.0.1
- Build Date: 2025-11-22
- Compiler: Apple Clang 17.0
- JUCE Version: 7.x
- C++ Standard: C++20
- Architecture: arm64 (Apple Silicon optimized)

---

## Bug Fixes Summary

| Version | Issue | Fix |
|---------|-------|-----|
| 1.0.1 | Crash on plugin open | Added null check in resized() |
| 1.0.1 | Wrong vendor name | Changed to Sonicdesign |

---

## Future Versions

See `TODO.md` for planned features.

### v1.1 (Planned)
- Fix all compiler warnings
- Add reverb and delay effects
- Drag & drop sample loading
- Preset browser

### v1.5 (Planned)
- Full sequencer implementation
- Sample editor with waveform
- Per-pad EQ

### v2.0 (Planned)
- Pattern chaining
- Effects chain per pad
- Cloud integration
