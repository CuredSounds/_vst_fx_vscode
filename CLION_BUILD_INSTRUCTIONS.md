# Building in CLion (macOS 15 Workaround)

## 🚧 Issue

JUCE has API compatibility issues with macOS 15. The `CGWindowListCreateImage` API was removed in macOS 15, breaking the build.

## ✅ Solution Options

### **Option 1: Build for macOS 14 Target (Recommended)**

Add this to your CMakeLists.txt:

```cmake
# Force macOS 14 SDK
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS deployment version")
```

Then in CLion:
1. **Tools → CMake → Reset Cache and Reload Project**
2. **Build → Build Project**

---

### **Option 2: Disable Problematic JUCE Features**

The issue is in `juce_gui_basics` (screenshot capture). If you don't need GUI screenshots, disable it:

Add to CMakeLists.txt before `add_subdirectory(JUCE-master)`:

```cmake
# Disable screenshot API that breaks on macOS 15
add_compile_definitions(JUCE_DISABLE_NATIVE_FILECHOOSERS=1)
```

---

### **Option 3: Patch JUCE Manually**

Edit: `JUCE-master/modules/juce_gui_basics/native/juce_Windowing_mac.mm`

Find line ~532:
```cpp
CGImageRef screenShot = CGWindowListCreateImage (CGRectNull, ...
```

Replace with:
```cpp
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 150000
CGImageRef screenShot = CGWindowListCreateImage (CGRectNull, ...
#else
CGImageRef screenShot = nullptr; // Disabled on macOS 15+
#endif
```

---

### **Option 4: Use Latest JUCE (Best Long-Term)**

JUCE team is working on macOS 15 support. Update to latest:

```bash
cd JUCE-master
git fetch origin
git checkout develop
cd ..
rm -rf build
```

Then reload CMake in CLion.

---

## 🎯 Quick Fix for Your Project

Since your plugins don't use screenshot features, **Option 1** is fastest:

1. Add this line to `CMakeLists.txt` after line 3:
   ```cmake
   set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS deployment version")
   ```

2. In CLion: **Tools → CMake → Reset Cache and Reload Project**

3. Build!

Your plugins will work on macOS 14+ (which includes macOS 15).

---

## 🏗️ CLion Build Steps

1. **Open Project**: `File → Open → Select _BC_vst_comp folder`
2. **Wait for CMake**: CLion auto-detects CMakeLists.txt
3. **Select Build Configuration**: Top-right dropdown → Debug or Release
4. **Build**:
   - `Cmd+F9` (Build Project)
   - Or click 🔨 hammer icon
5. **Find Plugin**:
   - VST3: `build/BCCompressor_artefacts/VST3/BC Compressor.vst3`
   - AU: `build/BCCompressor_artefacts/AU/BC Compressor.component`
   - Standalone: `build/BCCompressor_artefacts/BCCompressor`

---

## 🐛 If Build Still Fails

### Error: "juceaide build failed"

This is the macOS 15 issue. Use **Option 1** above.

### Error: "cannot find JuceHeader.h"

Run: **Tools → CMake → Reset Cache and Reload Project**

### Error: Missing JUCE modules

Make sure `JUCE-master/` folder exists and contains:
```
JUCE-master/
├── CMakeLists.txt
├── modules/
│   ├── juce_audio_basics/
│   ├── juce_audio_processors/
│   ├── juce_dsp/
│   └── ...
```

---

## 🎵 Testing Your Plugin

### Standalone App
```bash
./build/BCCompressor_artefacts/Debug/BCCompressor
```

### Install VST3 to System
```bash
cp -r build/BCCompressor_artefacts/Debug/VST3/BC\ Compressor.vst3 \
    ~/Library/Audio/Plug-Ins/VST3/
```

Then open your DAW and scan for plugins!

---

## 💡 Pro Tips

1. **Debug Build** for development (faster compile, has symbols)
2. **Release Build** for distribution (optimized, smaller)
3. **Rebuild Clean**: `Build → Rebuild Project` if things get weird
4. **CMake Output**: View errors in **CMake** tab (bottom of CLion)

---

## 🚀 Alternative: Simple Build Script

Create `build.sh`:
```bash
#!/bin/bash
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
cmake --build build --config Release
```

Run: `chmod +x build.sh && ./build.sh`

---

## ✅ Recommended Fix

**Just add this to CMakeLists.txt line 4:**

```cmake
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum macOS deployment version")
```

Then reload CMake. Done! 🎉
