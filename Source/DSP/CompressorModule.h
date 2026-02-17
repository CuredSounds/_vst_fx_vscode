#pragma once

#include <JuceHeader.h>
#include "EffectModule.h"

//==============================================================================
/**
 * Modular Compressor DSP Unit
 *
 * Implements the EffectModule interface for use in multi-FX hosts.
 * Can be:
 * - Used standalone in a VST3 wrapper (this project)
 * - Integrated into a multi-FX host (Snap Heap style)
 * - Chained with other effect modules
 *
 * Design principles:
 * - No AudioProcessor dependencies
 * - Real-time safe (no allocations in process())
 * - Fully implements EffectModule interface
 * - Sample-rate independent
 */
class CompressorModule : public EffectModule
{
public:
    //==============================================================================
    // Legacy parameter struct (for backward compatibility with standalone plugin)
    struct Parameters
    {
        float thresholdDb = -20.0f;    // -60 to 0 dB
        float ratio = 4.0f;             // 1:1 to 20:1
        float attackMs = 5.0f;          // 0.1 to 100 ms
        float releaseMs = 100.0f;       // 10 to 1000 ms
        float kneeDb = 0.0f;            // 0 to 12 dB (hard to soft)
        float makeupDb = 0.0f;          // -12 to +12 dB
        float mixPercent = 100.0f;      // 0 to 100%
        bool useSidechain = false;
        float sidechainHpfFreq = 20.0f; // Hz
        bool lookaheadEnabled = false;
        float lookaheadMs = 5.0f;    // 0..100 ms
        bool oversampleEnabled = false;
        int oversampleFactor = 1; // 1,2,4
    };

    //==============================================================================
    CompressorModule();
    ~CompressorModule() override = default;

    //==============================================================================
    // EffectModule interface implementation
    Type getType() const override { return Type::Compressor; }
    juce::String getName() const override { return "BC Compressor"; }
    juce::String getIdentifier() const override { return "comp"; }

    void prepare (double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer,
                  const juce::AudioBuffer<float>* sidechainBuffer = nullptr) override;

    int getNumParameters() const override { return 13; }
    ParameterInfo getParameterInfo (int index) const override;
    void setParameterNormalized (int index, float normalizedValue) override;
    float getParameterNormalized (int index) const override;

    juce::ValueTree saveState() const override;
    void loadState (const juce::ValueTree& state) override;

    bool supportsSidechain() const override { return true; }
    MeterData getMeterData() const override;

    /** Current processing latency introduced by lookahead (samples) */
    int getLatencySamples() const override { return lookaheadDelaySamples; }

    //==============================================================================
    // Legacy interface (for standalone plugin wrapper)
    void process (juce::AudioBuffer<float>& mainBuffer,
                  const juce::AudioBuffer<float>* sidechainBuffer,
                  const Parameters& params);

    float getGainReductionDb() const { return currentGainReductionDb; }

private:
    //==============================================================================
    /** Compute gain reduction for a given input level */
    float computeGainReduction (float inputDb, const Parameters& params) const;

    /** Smooth parameter changes to avoid zipper noise */
    void updateSmoothers (const Parameters& params);

    //==============================================================================
    double sampleRate = 44100.0;
    int numChannels = 2;
    int maxBlockSize = 0;

    // Envelope follower state per channel
    std::vector<float> envelopeState;

    // Sidechain filter and scratch buffer
    juce::dsp::IIR::Filter<float> sidechainFilter;
    juce::AudioBuffer<float> sidechainScratchBuffer;

    // Smoothed parameters (for avoiding zipper noise)
    juce::SmoothedValue<float> smoothedThreshold;
    juce::SmoothedValue<float> smoothedRatio;
    juce::SmoothedValue<float> smoothedMakeup;
    juce::SmoothedValue<float> smoothedMix;

    // Attack/Release coefficients (computed from ms values)
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Current GR for metering
    std::atomic<float> currentGainReductionDb { 0.0f };

    // Level metering (for UI display)
    std::atomic<float> currentInputLevelDb { -80.0f };
    std::atomic<float> currentOutputLevelDb { -80.0f };

    // Dry buffer for parallel compression
    juce::AudioBuffer<float> dryBuffer;

    // Lookahead buffer (for lookahead compression)
    juce::AudioBuffer<float> lookaheadBuffer;
    int lookaheadBufferSize = 0;
    int lookaheadWritePos = 0;
    int lookaheadDelaySamples = 0; // current lookahead in samples
    float maxLookaheadMs = 100.0f; // maximum supported lookahead (ms)

    // Oversampling helper (prepared when enabled)
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // Current parameter state (for EffectModule interface)
    Parameters currentParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorModule)
};
