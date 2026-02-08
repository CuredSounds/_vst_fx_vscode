#include "SaturationModule.h"

//==============================================================================
SaturationModule::SaturationModule()
{
}

//==============================================================================
void SaturationModule::prepare (double newSampleRate, int maxBlockSize, int newNumChannels)
{
    sampleRate = newSampleRate;
    numChannels = newNumChannels;
    juce::ignoreUnused (maxBlockSize);
}

void SaturationModule::reset()
{
    // Stateless effect - nothing to reset
}

//==============================================================================
void SaturationModule::process (juce::AudioBuffer<float>& buffer,
                                const juce::AudioBuffer<float>* sidechainBuffer)
{
    juce::ignoreUnused (sidechainBuffer);

    if (bypassed)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numProcessChannels = std::min (numChannels, buffer.getNumChannels());

    if (numSamples == 0 || numProcessChannels == 0)
        return;

    // Process all samples
    for (int channel = 0; channel < numProcessChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = saturateSample (channelData[sample], currentParams.drive);
        }
    }
}

//==============================================================================
EffectModule::ParameterInfo SaturationModule::getParameterInfo (int index) const
{
    if (index == 0)
        return { "drive", "Drive", 0.0f, 1.0f, 0.0f, "", false };
    return {};
}

void SaturationModule::setParameterNormalized (int index, float normalizedValue)
{
    if (index == 0)
        currentParams.drive = normalizedValue;
}

float SaturationModule::getParameterNormalized (int index) const
{
    if (index == 0)
        return currentParams.drive;
    return 0.0f;
}

juce::ValueTree SaturationModule::saveState() const
{
    juce::ValueTree state ("SaturationModule");
    state.setProperty ("drive", currentParams.drive, nullptr);
    return state;
}

void SaturationModule::loadState (const juce::ValueTree& state)
{
    if (state.hasType ("SaturationModule"))
    {
        currentParams.drive = state.getProperty ("drive", 0.0f);
    }
}
