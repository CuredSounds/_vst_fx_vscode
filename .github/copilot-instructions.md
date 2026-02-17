## Quick context for AI coding agents

This repo is a JUCE-based audio plugin (BC Compressor) built with CMake and tested with Catch2/CTest.
Key areas to read before making changes:

- `Source/PluginProcessor.{h,cpp}` — the VST/AU wrapper: creates APVTS parameters, bridges to the DSP module, exposes compressor module for metering.
- `Source/DSP/CompressorModule.{h,cpp}` — the real-time DSP implementation. Design goals: no AudioProcessor dependencies, real-time safe (no allocations in process()), sample-rate independent.
- `Tests/` — unit tests scaffolded with Catch2 and run via CTest.
- `JUCE-master/` — vendored JUCE; keep changes minimal here. The project expects this directory to be present.

## Architecture summary (big picture)

- Audio wrapper (JUCE AudioProcessor) ⇄ APVTS parameters ⇄ CompressorModule (pure DSP).
- CompressorModule implements `EffectModule` and provides both a legacy `Parameters` struct used by the wrapper and a ValueTree-based save/load interface for module state.
- Sidechain support: plugin registers a sidechain bus and CompressorModule supports sidechain buffers.
- Build artifacts are produced under `build/BCCompressor_artefacts/` (VST3/AU/Standalone subfolders).

## Concrete patterns and conventions (use these examples)

- Parameter naming: APVTS keys are strings like `threshold`, `ratio`, `attack`, `release`, `mix`, `sidechain`, `sc_hpf`, etc. See `createParameterLayout()` in `Source/PluginProcessor.cpp`.
- Reading params for DSP: wrapper calls `getCurrentParameters()` which reads `apvts.getRawParameterValue("<name>")->load()` and populates `CompressorModule::Parameters`.
- DSP safety: avoid heap allocations or any non-real-time-safe calls inside `CompressorModule::process`.
- Latency: lookahead sets latency via `setLatencySamples(compressorModule.getLatencySamples())` in `prepareToPlay()`; preserve that contract when changing lookahead behavior.
- State: plugin saves APVTS state via XML in `getStateInformation()` and restores in `setStateInformation()`; CompressorModule also implements `saveState()` / `loadState()` with `juce::ValueTree`.

## Build / test / debug workflows (explicit commands)

- Fast local build (from repo root):

  cmake -B build -DCMAKE_BUILD_TYPE=Debug
  cmake --build build

- Release build:

  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release -j8

- CLion: open the repo root. If JUCE API breakages occur on macOS 15, set

  set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS deployment version")

  (recommended place: top-level `CMakeLists.txt` as suggested by `CLION_BUILD_INSTRUCTIONS.md`).

- Run unit tests (Catch2 / CTest):

  cmake -S Tests -B Tests/build
  cmake --build Tests/build
  cd Tests/build && ctest --output-on-failure

- Run the standalone executable (after build):

  ./build/BCCompressor_artefacts/Debug/Standalone/BC\ Compressor.app/Contents/MacOS/BC\ Compressor

- Install VST3 for quick DAW testing:

  cp -r "build/BCCompressor_artefacts/Debug/VST3/BC Compressor.vst3" ~/Library/Audio/Plug-Ins/VST3/

## Integration / external deps

- JUCE is vendored under `JUCE-master/`. Do not expect network fetches during CMake.
 - Tests use Catch2; see `Tests/CMakeLists.txt` and `Tests/build/` usage above.

## Cross-platform & host notes

- Targets: macOS and Windows (VST3). AU is macOS-only. When adding platform-specific code, keep platform guards localized and avoid touching JUCE internals in `JUCE-master`.
- DAWs to validate manually: Nuendo, Ableton Live, Bitwig. Validation checklist: plugin loads, GUI responds, parameter automation recorded/replayed, CPU/latency within acceptable range.
- CI currently runs on `ubuntu-latest` and `macos-latest` (`.github/workflows/ci.yml`). Consider adding `windows-latest` to the matrix to catch platform-specific build issues for VST3.

## Things an AI agent should NOT change without human confirmation

- Large edits inside `JUCE-master/` (updating JUCE version) — these are high-risk and should be proposed first.
- Real-time code patterns in `Source/DSP/*` that allocate or call blocking APIs in `process()`.
- Changing APVTS parameter keys (string names) without updating the plugin editor and tests that reference them.

## Small safe improvements an AI agent may attempt proactively

- Add or update unit tests in `Tests/` for DSP math (use the existing Catch2 scaffold).
- Add small doc snippets or inline TODOs that explain parameter ranges (use values from `createParameterLayout`).
- Add CI-friendly scripts that wrap the explicit cmake + build + ctest sequences (keep changes outside `JUCE-master`).

## Where to look for examples in this repo

- Parameter layout & APVTS → `Source/PluginProcessor.cpp` (createParameterLayout, getCurrentParameters)
- DSP implementation & real-time rules → `Source/DSP/CompressorModule.h/.cpp`
- Effect module factory patterns → `Source/DSP/EffectModule.*`
- Build & CLion tips → `CLION_BUILD_INSTRUCTIONS.md`, `CLION_QUICK_START.md`
- Tests → `Tests/` and `TESTING.md`

If anything here is unclear or you want the instructions to reflect more opinionated policies (for example: branch naming, commit message format, or preferred unit-test patterns), tell me which areas to expand and I'll iterate.
