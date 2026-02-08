#pragma once

#include <JuceHeader.h>
#include "EffectModule.h"

//==============================================================================
/**
 * Modular Saturation/Distortion DSP Unit
 *
 * Features:
 * - Tanh waveshaping for smooth saturation
 * - Drive control (1x to 20x gain)
 * - Auto-gain compensation
 * - Real-time safe
 */
class SaturationModule : public EffectModule
{
public:
    //==============================================================================
    SaturationModule();
    ~SaturationModule() override = default;

    //==============================================================================
    // EffectModule interface
    Type getType() const override { return Type::Distortion; }
    juce::String getName() const override { return "BC Saturation"; }
    juce::String getIdentifier() const override { return "saturation"; }

    void prepare (double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer,
                  const juce::AudioBuffer<float>* sidechainBuffer = nullptr) override;

    int getNumParameters() const override { return 1; }
    ParameterInfo getParameterInfo (int index) const override;
    void setParameterNormalized (int index, float normalizedValue) override;
    float getParameterNormalized (int index) const override;

    juce::ValueTree saveState() const override;
    void loadState (const juce::ValueTree& state) override;

private:
    //==============================================================================
    struct Parameters
    {
        float drive = 0.0f; // 0 to 1 (normalized)
    } currentParams;

    // Saturation function
    static inline float saturateSample (float x, float drive)
    {
        const float minGain = 1.0f;
        const float maxGain = 20.0f;
        const float gain = minGain + drive * (maxGain - minGain);

        const float y = std::tanh (gain * x);
        const float normalize = 1.0f / std::tanh (maxGain);
        return y * normalize;
    }

    double sampleRate = 44100.0;
    int numChannels = 2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturationModule)
};
