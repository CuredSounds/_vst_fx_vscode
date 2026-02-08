# CLion Quick Start Guide

## 🔧 Reload CMake in CLion (Updated Location)

The CMake reload option moved in recent CLion versions. Here are all the ways to do it:

### **Method 1: CMake Panel (Easiest)**
1. Look at the **bottom** of CLion window
2. Find the **"CMake"** tab (next to Build, Run, Terminal, etc.)
3. Click the **🔄 Reload** icon in that panel
4. Or right-click in the CMake panel → **Reload CMake Project**

### **Method 2: File Menu**
```
File → Reload CMake Project
```

### **Method 3: Right-Click CMakeLists.txt**
1. In the **Project** panel (left side)
2. Right-click on `CMakeLists.txt`
3. Select **Reload CMake Project**

### **Method 4: Keyboard Shortcut**
Press: `Cmd+Shift+A` (Find Action)
Type: "reload cmake"
Select: **Reload CMake Project**

---

## 🏗️ How to Build

### **Option 1: Build Menu**
```
Build → Build Project
```
Or press `Cmd+F9`

### **Option 2: Toolbar**
Click the 🔨 **Build** button in the top toolbar

### **Option 3: Run Configuration**
1. Top-right dropdown: Select **BCCompressor** or **BCCompressor_Standalone**
2. Click the ▶️ green play button

---

## 🔍 Check Build Status

Look at the **bottom panel** for:
- **CMake** tab - Shows CMake configuration logs
- **Build** tab - Shows compilation progress
- **Messages** tab - Shows errors/warnings

---

## 📦 Where Are My Built Plugins?

After successful build:

```
build/BCCompressor_artefacts/Debug/
├── VST3/BC Compressor.vst3       ← Your VST3 plugin
├── AU/BC Compressor.component     ← Your AU plugin
└── Standalone/BC Compressor.app   ← Standalone app
```

---

## 🚀 Quick Build Commands

If CLion UI is confusing, just use Terminal inside CLion:

```bash
# Clean rebuild
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Or Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j8
```

---

## 🎯 First Time Setup Checklist

1. ✅ **Open Project**: `File → Open` → Select `_BC_vst_comp` folder
2. ✅ **Wait for Indexing**: CLion will scan files (progress bar at bottom)
3. ✅ **Reload CMake**: Use any method above
4. ✅ **Select Configuration**: Top-right dropdown → **Debug** or **Release**
5. ✅ **Build**: Press `Cmd+F9`

---

## 🐛 Common Issues

### "Cannot find CMake"
- Go to: **Preferences → Build, Execution, Deployment → CMake**
- Make sure CMake is detected (should show path like `/usr/local/bin/cmake`)

### "JUCE not found"
- Make sure `JUCE-master/` folder exists in your project
- Contains `CMakeLists.txt` and `modules/` folder

### Build hangs at "Building juceaide"
- This is the macOS 15 issue
- My fix should help, but if it still fails:
  ```bash
  cmake -B build -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
  ```

### "No CMake Project loaded"
- Right-click `CMakeLists.txt` → **Load CMake Project**

---

## ✅ Verify Everything Works

Run this in Terminal (inside CLion: `View → Tool Windows → Terminal`):

```bash
# Check JUCE exists
ls JUCE-master/CMakeLists.txt

# Check source files exist
ls Source/DSP/*.cpp

# Try manual build
cmake -B build -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
```

If all those work, CLion should build fine!

---

## 🎵 Test Your Plugin

### Run Standalone:
```bash
./build/BCCompressor_artefacts/Debug/Standalone/BC\ Compressor.app/Contents/MacOS/BC\ Compressor
```

### Install to System:
```bash
# VST3
cp -r "build/BCCompressor_artefacts/Debug/VST3/BC Compressor.vst3" \
    ~/Library/Audio/Plug-Ins/VST3/

# Audio Unit
cp -r "build/BCCompressor_artefacts/Debug/AU/BC Compressor.component" \
    ~/Library/Audio/Plug-Ins/Components/
```

Then rescan plugins in your DAW!

---

## 💡 Pro Tip

Create a CLion **External Tool** for quick installs:

1. **Preferences → Tools → External Tools**
2. Click **+** (Add)
3. **Name:** Install Plugin
4. **Program:** `/bin/bash`
5. **Arguments:**
   ```
   -c "cp -r '$ProjectFileDir$/build/BCCompressor_artefacts/Debug/VST3/BC Compressor.vst3' ~/Library/Audio/Plug-Ins/VST3/"
   ```
6. **Working directory:** `$ProjectFileDir$`

Now you can right-click project → **External Tools → Install Plugin**!

---

## 🎉 You're Ready!

Your project is all set up. Just:
1. Reload CMake (any method above)
2. Build (`Cmd+F9`)
3. Rock out! 🎸
