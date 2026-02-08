#pragma once

#include <JuceHeader.h>
#include "EffectModule.h"

//==============================================================================
/**
 * Effect Chain - Multi-FX Host System
 *
 * Manages a serial chain of effect modules (like Kilohearts Snap Heap).
 * Effects are processed in order, with audio passing through each module.
 *
 * Features:
 * - Dynamic effect loading/unloading
 * - Automatic latency compensation
 * - Per-effect bypass
 * - State save/load (presets)
 * - Real-time safe processing
 */
class EffectChain
{
public:
    //==============================================================================
    struct Slot
    {
        std::unique_ptr<EffectModule> effect;
        bool bypassed = false;
        juce::String slotId;  // Unique identifier for this slot

        Slot() = default;
        Slot (std::unique_ptr<EffectModule> fx, juce::String id)
            : effect (std::move (fx)), slotId (std::move (id)) {}
    };

    //==============================================================================
    EffectChain();
    ~EffectChain() = default;

    //==============================================================================
    /** Prepares all effects in the chain */
    void prepare (double sampleRate, int maxBlockSize, int numChannels);

    /** Resets all effects */
    void reset();

    /**
     * Process audio through the entire chain
     *
     * @param buffer Main audio buffer (in-place)
     * @param sidechainBuffer Optional sidechain (passed to all effects)
     */
    void process (juce::AudioBuffer<float>& buffer,
                  const juce::AudioBuffer<float>* sidechainBuffer = nullptr);

    //==============================================================================
    /** Add effect to end of chain */
    void addEffect (std::unique_ptr<EffectModule> effect);

    /** Insert effect at specific position */
    void insertEffect (int position, std::unique_ptr<EffectModule> effect);

    /** Remove effect at position */
    void removeEffect (int position);

    /** Move effect from one position to another */
    void moveEffect (int fromPosition, int toPosition);

    /** Clear all effects */
    void clearChain();

    //==============================================================================
    /** Get number of effects in chain */
    int getNumEffects() const { return static_cast<int> (effectSlots.size()); }

    /** Get effect at position (returns nullptr if invalid) */
    EffectModule* getEffect (int position) const;

    /** Get slot at position */
    Slot* getSlot (int position);

    //==============================================================================
    /** Bypass/unbypass effect at position */
    void setEffectBypassed (int position, bool shouldBeypass);

    /** Check if effect at position is bypassed */
    bool isEffectBypassed (int position) const;

    //==============================================================================
    /** Save entire chain state */
    juce::ValueTree saveState() const;

    /** Load entire chain state */
    void loadState (const juce::ValueTree& state);

    //==============================================================================
    /** Get total latency of chain in samples */
    int getTotalLatencySamples() const;

private:
    //==============================================================================
    std::vector<Slot> effectSlots;

    double currentSampleRate = 44100.0;
    int currentMaxBlockSize = 512;
    int currentNumChannels = 2;

    // Generate unique slot IDs
    juce::String generateSlotId();
    int nextSlotId = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectChain)
};
