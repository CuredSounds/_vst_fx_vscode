# _BC_delay

A high-quality VST3 and Standalone delay audio effect built with the JUCE framework.

## Features

- **Standard Controls**: Time, Feedback, and Mix.
- **Feedback Filters**: Integrated Low-Pass and High-Pass filters in the feedback loop.
- **Stereo Ping-Pong**: Cross-fed echoes for wide stereo imaging.
- **Analog Character**: Tape-style LFO modulation (Wow & Flutter) and Saturation (warmth).
- **Real-time Visualizer**: Integrated output waveform visualizer to see echoes in action.

## Project Structure

- `Source/`: Plugin source code (Processor and Editor).
- `JUCE/`: JUCE framework (required for building).
- `vst3sdk/`: VST3 SDK.

## Building the Project

This project uses CMake. To build it:

1. Ensure you have CMake and a C++ compiler installed.
2. Open the project folder in your IDE (CLion, VS Code, etc.) or use the command line:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
