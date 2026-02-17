# Testing Plan

This document outlines a recommended manual and automated testing matrix for audio plugin validation.

## DAW & Platform Matrix

- Ableton Live (macOS, Windows)
- Logic Pro (macOS)
- Cubase (macOS, Windows)
- Reaper (macOS, Windows)

## Test scenarios

1. Plugin load: open plugin in plugin host/plugin chain.
2. GUI: interact with knobs, automation, parameter changes.
3. Audio path: feed audio and confirm audible processing, no clicks/pops.
4. CPU and stability: stress test with many instances and large buffer sizes.
5. Presets: save and recall presets; test cross-version compatibility.
6. Automation: write automation and verify parameter recall.

## Automated tests

- Unit tests (CTEST) for DSP functions and utility code are recommended. Avoid testing DAW behaviour in CI.
- Use integration tests locally for plugin load validation (manual or scripted hosts).

## Suggested test checklist for each DAW/platform

- [ ] Open a fresh project
- [ ] Insert the plugin on an audio track
- [ ] Confirm GUI appears and responds
- [ ] Play audio through it and verify processing
- [ ] Save and reload project
- [ ] Capture CPU usage under stress


