#pragma once

#include <JuceHeader.h>
#include "EffectModule.h"

//==============================================================================
/**
 * Modular Delay DSP Unit
 *
 * Features:
 * - Stereo delay with ping-pong mode
 * - Feedback loop with LP/HP filters
 * - Tape-style LFO modulation (Wow & Flutter)
 * - Saturation for analog character
 * - Real-time safe
 */
class DelayModule : public EffectModule
{
public:
    //==============================================================================
    DelayModule();
    ~DelayModule() override = default;

    //==============================================================================
    // EffectModule interface
    Type getType() const override { return Type::Delay; }
    juce::String getName() const override { return "BC Delay"; }
    juce::String getIdentifier() const override { return "delay"; }

    void prepare (double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer,
                  const juce::AudioBuffer<float>* sidechainBuffer = nullptr) override;

    int getNumParameters() const override { return 9; }
    ParameterInfo getParameterInfo (int index) const override;
    void setParameterNormalized (int index, float normalizedValue) override;
    float getParameterNormalized (int index) const override;

    juce::ValueTree saveState() const override;
    void loadState (const juce::ValueTree& state) override;

private:
    //==============================================================================
    struct Parameters
    {
        float timeSeconds = 0.5f;       // 0 to 2s
        float feedback = 0.5f;          // 0 to 0.95
        float mix = 0.5f;               // 0 to 1
        float lpFreq = 20000.0f;        // 20 to 20000 Hz
        float hpFreq = 20.0f;           // 20 to 20000 Hz
        bool pingPong = false;
        float modRate = 1.0f;           // 0.1 to 10 Hz
        float modDepth = 0.002f;        // 0 to 0.01s
        float saturation = 1.0f;        // 0 to 2
    } currentParams;

    //==============================================================================
    double sampleRate = 44100.0;
    int numChannels = 2;

    // DSP components (stereo)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine[2];
    juce::dsp::Oscillator<float> lfo[2];
    juce::dsp::StateVariableTPTFilter<float> lpFilter[2];
    juce::dsp::StateVariableTPTFilter<float> hpFilter[2];

    juce::LinearSmoothedValue<float> timeSmoother;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayModule)
};
