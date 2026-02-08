#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Base Effect Module Interface
 *
 * Abstract base class for all modular audio effects in the multi-FX system.
 * Similar to Kilohearts Snap Heap architecture where each effect is a
 * self-contained unit that can be:
 * - Chained together
 * - Loaded/unloaded dynamically
 * - Hosted in multiple contexts
 * - Processed independently
 *
 * Design Principles:
 * - Real-time safe (no allocations in process())
 * - Sample-rate independent
 * - No AudioProcessor dependencies
 * - Serializable state
 */
class EffectModule
{
public:
    //==============================================================================
    /** Effect Type Enum - for factory instantiation */
    enum class Type
    {
        Compressor,
        EQ,
        Distortion,
        Reverb,
        Delay,
        Chorus,
        Filter,
        // Add more types as you build them
        Unknown
    };

    //==============================================================================
    virtual ~EffectModule() = default;

    //==============================================================================
    /** Returns the effect type */
    virtual Type getType() const = 0;

    /** Returns human-readable name (e.g., "BC Compressor") */
    virtual juce::String getName() const = 0;

    /** Returns short identifier (e.g., "comp") */
    virtual juce::String getIdentifier() const = 0;

    //==============================================================================
    /** Prepares the module for processing */
    virtual void prepare (double sampleRate, int maxBlockSize, int numChannels) = 0;

    /** Resets internal state (envelopes, buffers, etc.) */
    virtual void reset() = 0;

    /**
     * Process audio block
     *
     * @param buffer Main audio buffer (in-place processing)
     * @param sidechainBuffer Optional sidechain input (can be nullptr)
     */
    virtual void process (juce::AudioBuffer<float>& buffer,
                         const juce::AudioBuffer<float>* sidechainBuffer = nullptr) = 0;

    //==============================================================================
    /** Returns number of parameters this effect has */
    virtual int getNumParameters() const = 0;

    /** Get parameter info by index */
    struct ParameterInfo
    {
        juce::String id;
        juce::String name;
        float minValue;
        float maxValue;
        float defaultValue;
        juce::String unit;  // "dB", "Hz", "ms", "%", etc.
        bool isBoolean;
    };

    virtual ParameterInfo getParameterInfo (int index) const = 0;

    /** Set parameter value by index (normalized 0-1) */
    virtual void setParameterNormalized (int index, float normalizedValue) = 0;

    /** Get parameter value by index (normalized 0-1) */
    virtual float getParameterNormalized (int index) const = 0;

    //==============================================================================
    /** Serialize state to ValueTree (for presets/saving) */
    virtual juce::ValueTree saveState() const = 0;

    /** Restore state from ValueTree */
    virtual void loadState (const juce::ValueTree& state) = 0;

    //==============================================================================
    /** Returns true if effect supports sidechain input */
    virtual bool supportsSidechain() const { return false; }

    /** Returns current processing latency in samples */
    virtual int getLatencySamples() const { return 0; }

    /**
     * Get metering/visualization data (optional)
     * Override to provide custom meter data (GR, RMS, spectrum, etc.)
     */
    struct MeterData
    {
        float inputLevel = 0.0f;
        float outputLevel = 0.0f;
        float gainReduction = 0.0f;  // For compressors
        // Add more metering fields as needed
    };

    virtual MeterData getMeterData() const { return {}; }

    //==============================================================================
    /** Enable/disable bypass */
    virtual void setBypass (bool shouldBeypass) { bypassed = shouldBeypass; }
    virtual bool isBypassed() const { return bypassed; }

protected:
    bool bypassed = false;
};

//==============================================================================
/**
 * Effect Module Factory
 *
 * Creates effect instances by type.
 * Register new effects here as you build them.
 */
class EffectModuleFactory
{
public:
    /** Create an effect module by type */
    static std::unique_ptr<EffectModule> createEffect (EffectModule::Type type);

    /** Get list of all available effect types */
    static std::vector<EffectModule::Type> getAvailableEffects();

    /** Get human-readable name for effect type */
    static juce::String getEffectName (EffectModule::Type type);
};
