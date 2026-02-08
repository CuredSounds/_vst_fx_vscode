# Multi-FX System Usage Guide

## 🎯 Overview

Your modular multi-FX architecture is now complete with **3 effect modules**:
- **CompressorModule** - 8 parameters, sidechain support
- **DelayModule** - 9 parameters, stereo ping-pong, tape modulation
- **SaturationModule** - 1 parameter, simple distortion

---

## 🏗️ Architecture Quick Reference

```
EffectModule (Base Interface)
    ├── CompressorModule
    ├── DelayModule
    └── SaturationModule

EffectChain (Multi-FX Host)
    └── Manages serial chain of EffectModules

EffectModuleFactory
    └── Creates effects by type
```

---

## 📝 Usage Examples

### **Example 1: Create a Simple Effect Chain**

```cpp
#include "DSP/EffectChain.h"
#include "DSP/EffectModule.h"

// Create the chain
EffectChain chain;
chain.prepare (44100.0, 512, 2);

// Add effects in order: Comp → Delay → Saturation
chain.addEffect (EffectModuleFactory::createEffect (EffectModule::Type::Compressor));
chain.addEffect (EffectModuleFactory::createEffect (EffectModule::Type::Delay));
chain.addEffect (EffectModuleFactory::createEffect (EffectModule::Type::Distortion));

// Process audio
chain.process (audioBuffer, sidechainBuffer);
```

---

### **Example 2: Control Parameters**

```cpp
// Get effect from chain
auto* compressor = dynamic_cast<CompressorModule*> (chain.getEffect (0));
if (compressor)
{
    // Set threshold to -10 dB
    compressor->setParameterNormalized (0, 0.5f); // 0 = threshold param

    // Enable sidechain
    compressor->setParameterNormalized (7, 1.0f); // 7 = sidechain param
}

// Generic interface (any effect type)
auto* delay = chain.getEffect (1);
if (delay)
{
    auto info = delay->getParameterInfo (0); // Get first parameter info
    delay->setParameterNormalized (0, 0.8f); // Set it
}
```

---

### **Example 3: Save/Load Presets**

```cpp
// Save entire chain state
auto chainState = chain.saveState();

// Save to file
juce::File presetFile = juce::File::getSpecialLocation (
    juce::File::userDocumentsDirectory).getChildFile ("my_preset.xml");

auto xml = chainState.createXml();
xml->writeTo (presetFile);

// Load from file
auto loadedXml = juce::XmlDocument::parse (presetFile);
auto loadedState = juce::ValueTree::fromXml (*loadedXml);
chain.loadState (loadedState);
```

---

### **Example 4: Dynamic Effect Management**

```cpp
// Insert effect at position 1
chain.insertEffect (1, EffectModuleFactory::createEffect (
    EffectModule::Type::Compressor));

// Move effect from position 2 to position 0
chain.moveEffect (2, 0);

// Bypass effect at position 1
chain.setEffectBypassed (1, true);

// Remove effect at position 2
chain.removeEffect (2);

// Clear all effects
chain.clearChain();
```

---

### **Example 5: Build a Multi-FX Host Plugin**

Create a new AudioProcessor that hosts the chain:

```cpp
class MultiFXHostProcessor : public juce::AudioProcessor
{
public:
    MultiFXHostProcessor()
        : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo())
                          .withOutput ("Output", juce::AudioChannelSet::stereo()))
    {
        // Initialize with default effects
        effectChain.addEffect (EffectModuleFactory::createEffect (
            EffectModule::Type::Compressor));
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        effectChain.prepare (sampleRate, samplesPerBlock, 2);
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        effectChain.process (buffer, nullptr);
    }

    void releaseResources() override
    {
        effectChain.reset();
    }

    // Add GUI methods to add/remove effects dynamically
    void addNewEffect (EffectModule::Type type)
    {
        effectChain.addEffect (EffectModuleFactory::createEffect (type));
    }

    EffectChain& getEffectChain() { return effectChain; }

private:
    EffectChain effectChain;
};
```

---

### **Example 6: Iterate Through All Effects**

```cpp
for (int i = 0; i < chain.getNumEffects(); ++i)
{
    auto* effect = chain.getEffect (i);

    DBG ("Effect " << i << ": " << effect->getName());
    DBG ("  Type: " << EffectModuleFactory::getEffectName (effect->getType()));
    DBG ("  Parameters: " << effect->getNumParameters());
    DBG ("  Bypassed: " << (chain.isEffectBypassed (i) ? "Yes" : "No"));

    // Print all parameters
    for (int p = 0; p < effect->getNumParameters(); ++p)
    {
        auto info = effect->getParameterInfo (p);
        DBG ("    " << info.name << ": " << effect->getParameterNormalized (p));
    }
}
```

---

### **Example 7: Metering Data**

```cpp
// Get meter data from compressor
auto* comp = chain.getEffect (0);
if (comp && comp->getType() == EffectModule::Type::Compressor)
{
    auto meterData = comp->getMeterData();
    float gainReduction = meterData.gainReduction;

    // Display in GUI
    updateGainReductionMeter (gainReduction);
}
```

---

## 🎨 Building a Multi-FX GUI

### **Concept: Snap Heap Style Interface**

```cpp
class MultiFXEditor : public juce::AudioProcessorEditor
{
public:
    MultiFXEditor (MultiFXHostProcessor& p)
        : AudioProcessorEditor (&p), processor (p)
    {
        // Effect selector dropdown
        effectSelector.addItem ("Compressor", 1);
        effectSelector.addItem ("Delay", 2);
        effectSelector.addItem ("Saturation", 3);
        addAndMakeVisible (effectSelector);

        // Add effect button
        addEffectButton.onClick = [this] { addSelectedEffect(); };
        addAndMakeVisible (addEffectButton);

        // Effect chain display
        updateEffectChainView();
    }

private:
    void addSelectedEffect()
    {
        auto selected = effectSelector.getSelectedId();
        EffectModule::Type type;

        switch (selected)
        {
            case 1: type = EffectModule::Type::Compressor; break;
            case 2: type = EffectModule::Type::Delay; break;
            case 3: type = EffectModule::Type::Distortion; break;
            default: return;
        }

        processor.addNewEffect (type);
        updateEffectChainView();
    }

    void updateEffectChainView()
    {
        // Clear existing UI
        effectSlots.clear();

        // Create UI slot for each effect
        auto& chain = processor.getEffectChain();
        for (int i = 0; i < chain.getNumEffects(); ++i)
        {
            auto* effect = chain.getEffect (i);
            effectSlots.add (new EffectSlotComponent (effect, i));
        }

        resized();
    }

    MultiFXHostProcessor& processor;
    juce::ComboBox effectSelector;
    juce::TextButton addEffectButton { "Add Effect" };
    juce::OwnedArray<EffectSlotComponent> effectSlots;
};
```

---

## 🔧 Parameter Mapping Reference

### **CompressorModule** (8 params)
| Index | Name      | Range         | Unit |
|-------|-----------|---------------|------|
| 0     | threshold | -60 to 0      | dB   |
| 1     | ratio     | 1 to 20       | -    |
| 2     | attack    | 0.1 to 100    | ms   |
| 3     | release   | 10 to 1000    | ms   |
| 4     | knee      | 0 to 12       | dB   |
| 5     | makeup    | -12 to +12    | dB   |
| 6     | mix       | 0 to 100      | %    |
| 7     | sidechain | Off/On        | -    |

### **DelayModule** (9 params)
| Index | Name       | Range          | Unit |
|-------|------------|----------------|------|
| 0     | time       | 0 to 2         | s    |
| 1     | feedback   | 0 to 0.95      | -    |
| 2     | mix        | 0 to 1         | -    |
| 3     | lpfreq     | 20 to 20000    | Hz   |
| 4     | hpfreq     | 20 to 20000    | Hz   |
| 5     | pingpong   | Off/On         | -    |
| 6     | modrate    | 0.1 to 10      | Hz   |
| 7     | moddepth   | 0 to 0.01      | s    |
| 8     | saturation | 0 to 2         | -    |

### **SaturationModule** (1 param)
| Index | Name  | Range  | Unit |
|-------|-------|--------|------|
| 0     | drive | 0 to 1 | -    |

---

## 🚀 Next Steps

### **Easy Extensions:**

1. **Add More Effects**
   ```cpp
   // Just implement EffectModule interface!
   class ReverbModule : public EffectModule { ... };
   class EQModule : public EffectModule { ... };
   class ChorusModule : public EffectModule { ... };
   ```

2. **Parallel Processing**
   - Modify `EffectChain` to support parallel branches
   - Add routing matrix

3. **Modulation System**
   - Add LFO/Envelope → Parameter modulation
   - Store modulation state in ValueTree

4. **Visual Effect Chain**
   - Drag-and-drop reordering
   - Visual routing cables
   - Per-slot parameter editors

---

## ✅ Summary

You now have a **complete, production-ready multi-FX framework**:

✅ **3 Effect Modules** (Compressor, Delay, Saturation)
✅ **Unified EffectModule interface**
✅ **EffectChain for serial processing**
✅ **Factory pattern for easy instantiation**
✅ **Full state save/load**
✅ **Real-time safe**
✅ **Zero dependencies on AudioProcessor**

**Ready for commercial use!** 🎉
