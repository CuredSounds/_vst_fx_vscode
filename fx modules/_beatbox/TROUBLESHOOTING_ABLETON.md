# Getting BC BeatBox to Show in Ableton Live

## ✅ Your Plugins ARE Installed

The build was successful and your plugins are here:
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/BC BeatBox.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/BC BeatBox.component`

Both are correctly compiled for Apple Silicon (M4).

---

## 🔧 Steps to Fix

### Step 1: Clear macOS Audio Cache (Already Done ✅)
```bash
killall -9 AudioComponentRegistrar
```

### Step 2: Clear Ableton's Plugin Cache

**Quit Ableton completely**, then run:
```bash
rm -rf ~/Library/Preferences/Ableton/*/Plugin*
```

Or manually delete these folders:
- `~/Library/Preferences/Ableton/Live 11.x.x/PluginCache/`
- `~/Library/Preferences/Ableton/Live 11.x.x/PluginFolder/`

### Step 3: Configure Ableton's Plugin Settings

1. **Open Ableton Live**
2. **Preferences** (⌘,)
3. Go to **Plug-Ins** tab
4. Make sure these are **ENABLED** (checked):
   - ☑️ **Use VST3 Plug-in System Folders**
   - ☑️ **Use Audio Units**
5. Verify the paths include:
   - VST3: `/Library/Audio/Plug-Ins/VST3`
   - AU: `/Library/Audio/Plug-Ins/Components`
6. Click **Rescan** button

### Step 4: Find Your Plugin

After rescanning:

1. Look in **Browser** (left panel)
2. Navigate to **Plug-ins**
3. Check these sections:
   - **Instruments** → Look for "BC BeatBox" or "BroadcastConcepts"
   - **Audio Units** → Manufacturer "BroadcastConcepts"
   - **VST3** → Look for "BC BeatBox"

---

## 🎯 Quick Test

### Try Loading via Browser

1. Create a new **MIDI Track**
2. Double-click it to select it
3. In Browser: **Plug-ins** → **Instruments**
4. Find **BC BeatBox**
5. **Drag** it onto the track or **double-click** to insert

### Search Function

1. Click the **search box** in Ableton's browser
2. Type: `beatbox` or `broadcast`
3. Should show up if installed correctly

---

## 🐛 If Still Not Showing

### Check Ableton's Plugin Log

1. Go to: `~/Library/Preferences/Ableton/Live X.X.X/`
2. Look for `Log.txt`
3. Search for "BeatBox" or errors

Or from terminal:
```bash
grep -i "beatbox\|broadcast" ~/Library/Preferences/Ableton/*/Log.txt
```

### Verify Plugin Format

Check if Ableton sees it:
```bash
# Check VST3
ls -la ~/Library/Audio/Plug-Ins/VST3/BC\ BeatBox.vst3/Contents/MacOS/

# Check AU
ls -la ~/Library/Audio/Plug-Ins/Components/BC\ BeatBox.component/Contents/MacOS/
```

Both should show a binary file called "BC BeatBox".

### Test with Another DAW

Try loading in another DAW to verify the plugin works:
- **Logic Pro** (AU support)
- **GarageBand** (AU support)
- **Reaper** (Both VST3 and AU)

### Rebuild in Release Mode

The Debug build might have issues. Try Release mode:

```bash
cd /Users/sonic.design/CLionProjects/_BC_BeatBox

# Clean
rm -rf cmake-build-debug

# Build Release instead
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release -j8

# Install manually
cp -r cmake-build-release/BeatBox_artefacts/Release/VST3/BC\ BeatBox.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/

cp -r cmake-build-release/BeatBox_artefacts/Release/AU/BC\ BeatBox.component \
      ~/Library/Audio/Plug-Ins/Components/

# Clear cache again
killall -9 AudioComponentRegistrar
```

Then restart Ableton.

---

## 🔍 Common Issues

### Issue 1: Ableton Not Scanning System Folders

**Fix**: In Preferences → Plug-ins, make sure "Use VST3 Plug-in System Folders" is checked.

### Issue 2: Plugin Shows as "Failed" or Greyed Out

**Possible causes**:
- Code signing issue (Debug builds)
- Missing dependencies
- Architecture mismatch (should be arm64 for M4)

**Fix**: Build in Release mode (see above)

### Issue 3: Plugin Crashes on Load

Check the logs:
```bash
# View Ableton crash logs
ls -lt ~/Library/Logs/DiagnosticReports/ | grep -i ableton | head -5

# View our plugin logs
tail -50 ~/Library/Logs/BeatBox/beatbox.log
```

---

## ✅ Expected Behavior

When working correctly:

1. **In Browser**: Shows under Plug-ins → Instruments → "BC BeatBox"
2. **Icon**: Should show generic plugin icon (we didn't add custom icon)
3. **Name**: "BC BeatBox" or "BC BEATBOX"
4. **Manufacturer**: BroadcastConcepts
5. **When Loaded**: Shows GUI with 16 drum pads in 4×4 grid

---

## 🎹 Once Loaded

### How to Use:

1. **Click pads** to trigger sounds
2. **Right-click pad** → Load Sample
3. **Switch modes**:
   - Synthesis (synth drums)
   - Sampling (your samples)
   - Hybrid (both layered)
4. **Adjust Master Gain** slider

### MIDI:
- Pads respond to MIDI notes C1-D#2 (notes 36-51)
- Pad 1 = C1 (36), Pad 2 = D1 (38), etc.

---

## 📞 Still Need Help?

If none of this works:

1. **Check build output** - share any errors
2. **Check Ableton version** - which version are you using?
3. **Check logs**:
   ```bash
   tail -50 ~/Library/Logs/BeatBox/beatbox.log
   ```
4. **Try Logic/GarageBand** - test if it's Ableton-specific

---

## 🚀 Next Steps After It Works

Once you see it in Ableton:

1. Load it on a MIDI track
2. Click some pads - you should hear synthesized drums
3. Right-click a pad and load a drum sample (WAV/AIFF)
4. Switch to "Hybrid" mode to layer synth + sample
5. Save your project - settings should persist

**Have fun making beats!** 🥁
