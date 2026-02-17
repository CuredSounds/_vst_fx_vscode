# _BC_vst_comp
basic compressor module

## Quickstart: Build & Test

This project now includes a minimal unit test scaffold using Catch2 and CTest.

### Run tests locally

```sh
cmake -S Tests -B Tests/build
cmake --build Tests/build
cd Tests/build
ctest --output-on-failure
```

## Platform & DAW notes

This plugin targets macOS and Windows and produces VST3 and (macOS-only) AU formats. Recommended platform notes:

- macOS (VST3, AU, Standalone): build using CMake/CLion per `CLION_BUILD_INSTRUCTIONS.md`. For macOS 15 compatibility set `CMAKE_OSX_DEPLOYMENT_TARGET` to `14.0` in `CMakeLists.txt` as described.

- Windows (VST3): build using Visual Studio generator or MSVC toolchain. Example (developer machine with Visual Studio 2022):

```bash
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

- DAWs to validate manually: Nuendo, Ableton Live, Bitwig. Install VST3 to your system plugin folder and verify load/parameter automation and CPU usage in each host.

## Contribution checklist

- Update `Tests/` with unit tests for any DSP changes (see existing Catch2 tests).
- If you change APVTS parameter keys (strings in `createParameterLayout()`), update the editor and any tests that reference them.
- Add a brief changelog entry in `CHANGELOG.md` for user-facing changes (parameter renames, preset format changes, etc.).

### Continuous Integration

All pushes and pull requests to `main` are automatically built and tested on macOS and Linux via GitHub Actions (`.github/workflows/ci.yml`).

### Adding more tests

Add new test files to `Tests/` and update `Tests/CMakeLists.txt` as needed. See [Catch2 documentation](https://github.com/catchorg/Catch2) for more info.
