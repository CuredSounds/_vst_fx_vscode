# BC BeatBox - TODO List & Feature Roadmap

## Priority 1 - Critical Fixes

### Bug Fixes
- [ ] **Fix Font constructor deprecation warnings**
  - Replace `juce::Font(float)` with `juce::Font(juce::FontOptions().withHeight(float))`
  - Files: `PluginEditor.cpp` (lines 141, 147, 169)
  - Severity: Low (just warnings, but should fix for future JUCE compatibility)

- [ ] **Fix unused parameter warnings**
  - `DrumSampler.cpp:301` - unused `padIndex` in `updateVoiceParameters()`
  - `PluginEditor.cpp:178` - unused `header` variable
  - Severity: Low (cleanup)

- [ ] **Add error handling for sample loading edge cases**
  - Handle corrupt audio files
  - Handle unsupported sample rates (auto-resample)
  - Handle very large files (>100MB)
  - Add user feedback for failures

---

## Priority 2 - Essential Features

### Audio Effects System

#### Built-in Effects
- [ ] **Reverb Effect**
  - Add `juce::dsp::Reverb` to AudioEngine
  - Create ReverbProcessor class
  - Add parameters: room size, damping, wet/dry, width
  - Add global reverb send per pad
  - UI: Reverb panel with controls

- [ ] **Delay Effect**
  - Implement `juce::dsp::DelayLine` based delay
  - Add parameters: time, feedback, wet/dry, ping-pong mode
  - Sync to host tempo (optional)
  - UI: Delay panel with controls

- [ ] **Distortion/Saturation**
  - Implement waveshaper distortion
  - Add parameters: drive, tone, wet/dry, type (soft/hard/tube)
  - Per-pad distortion amount
  - UI: Distortion panel

- [ ] **Master Effects Chain**
  - High-pass filter (remove DC offset/rumble)
  - Compressor/Limiter for output protection
  - Final gain stage
  - Metering (peak/RMS)

#### Per-Pad Effects
- [ ] **Individual EQ per pad**
  - 3-band EQ (low, mid, high)
  - Frequency and gain controls
  - UI: EQ display in pad detail view

- [ ] **Per-pad effect sends**
  - Reverb send amount (0-100%)
  - Delay send amount (0-100%)
  - Pre/post fader option
  - UI: Send knobs in pad detail

---

### Sequencer Implementation

- [ ] **16-Step Sequencer Engine**
  ```
  Priority: High
  Difficulty: Medium
  Estimated Time: 2-3 days
  ```
  - Implement `SequencerEngine` class
  - 16 steps per pattern
  - Multiple patterns (8-16 patterns)
  - Per-step velocity control
  - Step on/off toggle
  - Pattern playback with tempo sync
  - Transport controls (play/stop/record)

- [ ] **Sequencer GUI**
  - Replace placeholder `SequencerComponent`
  - 16x16 grid (16 pads × 16 steps)
  - Visual step indicators
  - Current step highlight
  - Pattern selector
  - BPM control
  - Swing control

- [ ] **Pattern Management**
  - Pattern save/load
  - Pattern copy/paste
  - Pattern clear
  - Pattern randomize
  - Pattern chain/song mode
  - Export pattern as MIDI file

- [ ] **Advanced Sequencer Features**
  - Variable pattern length (8, 16, 32 steps)
  - Per-track probability (70% chance to trigger)
  - Per-step micro-timing (shuffle/humanize)
  - Step repeat/roll
  - Automation per parameter

---

### Sample Management Improvements

- [ ] **Sample Editor**
  ```
  Priority: Medium
  Difficulty: High
  Estimated Time: 3-4 days
  ```
  - Waveform display component
  - Zoom in/out on waveform
  - Sample start/end point markers (drag to set)
  - Loop point markers
  - Normalize function
  - Reverse function (persist to pad)
  - Trim silence function

- [ ] **Multi-Layer Samples**
  - Load multiple samples per pad
  - Velocity-layer assignment (0-40, 41-80, 81-127)
  - Round-robin mode (cycle through samples)
  - Random mode (pick random sample)
  - UI: Layer manager per pad

- [ ] **Drag & Drop Support**
  - Drag audio files directly onto pads
  - Drag from OS file browser
  - Drag from DAW
  - Visual drop indicator

- [ ] **Sample Library Browser**
  - Browse folders of samples
  - Preview samples before loading
  - Favorites/tags system
  - Search functionality
  - Auto-categorize (kick, snare, etc.)

- [ ] **Sample Processing**
  - Auto-detect sample transients
  - Auto-trim silence
  - Pitch detection and auto-tune
  - Time-stretch (maintain pitch)
  - Format conversion utility

---

### Preset System

- [ ] **Preset Management**
  ```
  Priority: Medium
  Difficulty: Medium
  Estimated Time: 2 days
  ```
  - Preset browser UI
  - Factory preset library (10-20 kits)
  - User preset save/load
  - Preset categories (Hip-Hop, Techno, House, etc.)
  - Preset search/filter
  - Preset import/export
  - Preset sharing format

- [ ] **Factory Kits**
  - "808 Classic" kit
  - "909 Techno" kit
  - "Trap" kit
  - "Lo-Fi Hip-Hop" kit
  - "DnB" kit
  - "House" kit
  - "Industrial" kit
  - Record/source high-quality samples

---

## Priority 3 - UI/UX Improvements

### User Interface

- [ ] **Resizable GUI**
  - Allow window resize (within limits)
  - Scale UI elements proportionally
  - Remember window size in state
  - Handle high-DPI displays properly

- [ ] **Pad Detail View**
  - Click pad to open detail editor
  - Show: waveform, EQ curve, envelope shape
  - All pad parameters in one view
  - Sample file info (SR, bit depth, length)

- [ ] **Keyboard Shortcuts**
  - Trigger pads with computer keyboard
  - QWERTY layout maps to pads
  - Hold shift for higher velocity
  - Spacebar = play/stop sequencer

- [ ] **Visual Feedback**
  - VU meters on pads (show output level)
  - Clipping indicators
  - MIDI activity lights
  - Current sequencer step highlight
  - Waveform display on pads

- [ ] **Color Themes**
  - Dark mode (current)
  - Light mode
  - High contrast mode
  - Custom theme editor
  - Save theme preferences

- [ ] **Accessibility**
  - Screen reader support
  - Keyboard navigation for all controls
  - Tooltips on all UI elements
  - Larger text option

---

### Advanced Features

- [ ] **Undo/Redo System**
  - Undo sample loads
  - Undo parameter changes
  - Undo sequencer edits
  - Undo history (20-50 steps)
  - UI: Show undo/redo menu

- [ ] **MIDI Learn**
  - Right-click parameter → "MIDI Learn"
  - Listen for MIDI CC
  - Map CC to parameter
  - Visual indicator of mapped controls
  - Save mappings with project

- [ ] **Individual Pad Outputs**
  - Route each pad to separate DAW track
  - 16 stereo outputs (or 16 mono)
  - Configurable routing
  - Useful for parallel processing in DAW

- [ ] **Sidechain Trigger Output**
  - Send MIDI note when pad triggers
  - Use for sidechain compression in DAW
  - Configurable MIDI channel/note

- [ ] **Sample Rate Conversion**
  - Auto-resample loaded samples to project rate
  - Option to use native sample rate
  - Display sample rate mismatch warning

---

## Priority 4 - Performance & Optimization

### Performance

- [ ] **Optimize Voice Stealing**
  - Better voice stealing algorithm (oldest → quietest)
  - Crossfade when stealing to avoid clicks
  - Increase voice count option (32 → 64)

- [ ] **Sample Streaming**
  - Stream large samples from disk
  - Don't load entire file into RAM
  - Lookahead buffer for smooth playback
  - Useful for long samples (>10 seconds)

- [ ] **Multi-threading**
  - Separate DSP thread for effects
  - Background thread for file I/O
  - Lock-free audio queue
  - Better CPU utilization

- [ ] **CPU Usage Optimization**
  - Profile hot paths
  - Optimize filter calculations
  - SIMD optimization for DSP
  - Reduce allocations

---

## Priority 5 - Testing & Quality

### Testing

- [ ] **Unit Tests**
  ```
  Framework: Catch2 or Google Test
  ```
  - Test synthesis algorithms
  - Test sample playback accuracy
  - Test MIDI mapping
  - Test state serialization
  - Test parameter ranges
  - Test file loading edge cases

- [ ] **Integration Tests**
  - Test in multiple DAWs (Logic, Ableton, FL Studio, Reaper)
  - Test AU validation (auval)
  - Test VST3 validation
  - Test automation recording/playback
  - Test preset switching

- [ ] **Performance Tests**
  - CPU usage benchmarks
  - Maximum polyphony test
  - Memory leak detection
  - Load time benchmarks

---

## Priority 6 - Platform Support

### Windows Support

- [ ] **Windows Build**
  ```
  Status: CMake is ready, needs testing
  ```
  - Test build on Windows 10/11
  - Fix any Windows-specific issues
  - Test VST3 on Windows DAWs
  - Create Windows installer

- [ ] **Windows-specific Features**
  - ASIO support in standalone
  - Windows paths (backslashes)
  - Registry for preferences (or AppData)

### Linux Support (Future)

- [ ] **Linux Build** (Optional)
  - CMake configuration for Linux
  - LV2 plugin format
  - JACK audio support
  - Package for Ubuntu/Debian

---

## Priority 7 - Documentation & Help

### Documentation

- [ ] **User Manual**
  - PDF manual with screenshots
  - Getting started tutorial
  - Feature walkthrough
  - MIDI implementation chart
  - Troubleshooting section

- [ ] **Video Tutorials**
  - Quick start (5 min)
  - Building a kit (10 min)
  - Using the sequencer (10 min)
  - Advanced tips & tricks (15 min)

- [ ] **In-App Help**
  - Help menu with tooltips
  - Context-sensitive help
  - Link to online manual
  - Tutorial projects

- [ ] **Developer Documentation**
  - ✅ Already have ARCHITECTURE.md
  - ✅ Already have API.md
  - Add plugin-in-plugin guide (for extending)
  - Add DSP algorithm explanations

---

## Priority 8 - Advanced/Future Ideas

### Experimental Features

- [ ] **Machine Learning Integration**
  - Auto-classify samples (kick vs snare)
  - Generate drum patterns (AI sequencer)
  - Recommend similar sounds
  - Smart mixing suggestions

- [ ] **Cloud Features**
  - Cloud preset library
  - Share kits with community
  - Download sample packs
  - User accounts & sync

- [ ] **Modular Architecture**
  - Plugin-in-plugin system
  - Load custom DSP modules
  - Scripting support (Lua/JavaScript)
  - Custom GUI components

- [ ] **Collaboration Features**
  - Session sharing (work on same kit)
  - Version control for kits
  - Comments/annotations on patterns

- [ ] **Hardware Integration**
  - MIDI controller templates
  - Ableton Push integration
  - Native Instruments Maschine integration
  - iPad companion app

---

## Bug Tracker

### Known Issues
1. Font deprecation warnings (low priority)
2. Unused variable warnings (low priority)
3. No sample streaming for large files (medium priority)
4. Sequencer UI is placeholder (high priority)

### Reported Issues
(None yet - waiting for user testing)

---

## Version Planning

### Version 1.1 (Next Release)
**Target: 2-3 weeks**
- [ ] Fix all compiler warnings
- [ ] Add reverb effect
- [ ] Add delay effect
- [ ] Implement drag & drop
- [ ] Add preset browser
- [ ] Create 5-10 factory kits
- [ ] Add keyboard shortcuts

### Version 1.5 (Medium Term)
**Target: 1-2 months**
- [ ] Complete sequencer implementation
- [ ] Sample editor with waveform
- [ ] Multi-layer samples
- [ ] Per-pad EQ
- [ ] Individual outputs
- [ ] Windows build & testing

### Version 2.0 (Major Update)
**Target: 3-4 months**
- [ ] Pattern chaining/song mode
- [ ] Effects chain per pad
- [ ] Sample library browser
- [ ] Machine learning features
- [ ] Cloud integration
- [ ] Mobile companion app

---

## Community Ideas

(Space for user-requested features)

- [ ] _Your idea here!_

---

## Notes

### Design Decisions to Consider

1. **Sequencer vs. DAW Workflow**
   - Should sequencer be primary interface?
   - Or focus on live performance/DAW integration?

2. **Sample vs. Synthesis Balance**
   - Currently 50/50 - is this right?
   - Some users may want synthesis-only

3. **CPU vs. Features Trade-off**
   - More effects = more CPU
   - Need to maintain real-time performance

4. **Standalone vs. Plugin**
   - How much focus on standalone app?
   - Most users will use as plugin

### Technical Debt

- [ ] Refactor parameter management (too many atomic pointers)
- [ ] Consolidate MIDI handling logic
- [ ] Create BaseEffect class for effect plugins
- [ ] Abstract file I/O into dedicated manager
- [ ] Improve error handling consistency

---

## Contribution Guidelines

If you want to contribute to this project:

1. Pick a feature from this TODO
2. Create a branch: `git checkout -b feature/your-feature`
3. Follow existing code style
4. Add logging for debugging
5. Update documentation
6. Test thoroughly
7. Submit pull request

---

**Last Updated**: 2025-01-22
**Maintainer**: Broadcast Concepts
**Contributors**: [Add your name!]
