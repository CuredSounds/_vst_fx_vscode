#include "DelayModule.h"

//==============================================================================
DelayModule::DelayModule()
{
}

//==============================================================================
void DelayModule::prepare (double newSampleRate, int maxBlockSize, int newNumChannels)
{
    sampleRate = newSampleRate;
    numChannels = std::min (newNumChannels, 2); // Stereo max

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels = 1; // Mono processing per channel

    for (int i = 0; i < 2; ++i)
    {
        delayLine[i].prepare (spec);
        delayLine[i].setMaximumDelayInSamples (static_cast<int> (2.0 * sampleRate) + 2); // 2s max

        lfo[i].prepare (spec);
        lfo[i].initialise ([] (float x) { return std::sin (x); });

        lpFilter[i].prepare (spec);
        lpFilter[i].setType (juce::dsp::StateVariableTPTFilterType::lowpass);

        hpFilter[i].prepare (spec);
        hpFilter[i].setType (juce::dsp::StateVariableTPTFilterType::highpass);
    }

    timeSmoother.reset (sampleRate, 0.05);

    reset();
}

void DelayModule::reset()
{
    for (int i = 0; i < 2; ++i)
    {
        delayLine[i].reset();
        lfo[i].reset();
        lpFilter[i].reset();
        hpFilter[i].reset();
    }
    timeSmoother.reset (sampleRate, 0.05);
}

//==============================================================================
void DelayModule::process (juce::AudioBuffer<float>& buffer,
                           const juce::AudioBuffer<float>* sidechainBuffer)
{
    juce::ignoreUnused (sidechainBuffer);

    if (bypassed)
        return;

    const int bufferLength = buffer.getNumSamples();
    const int numProcessChannels = std::min (numChannels, buffer.getNumChannels());

    if (bufferLength == 0 || numProcessChannels == 0)
        return;

    // Update filter cutoffs and LFO rates
    for (int i = 0; i < numProcessChannels; ++i)
    {
        lpFilter[i].setCutoffFrequency (currentParams.lpFreq);
        hpFilter[i].setCutoffFrequency (currentParams.hpFreq);
        lfo[i].setFrequency (currentParams.modRate);
    }

    timeSmoother.setTargetValue (currentParams.timeSeconds);

    // Process samples
    for (int sample = 0; sample < bufferLength; ++sample)
    {
        float currentTime = timeSmoother.getNextValue();
        float samplesToDelay[2] = { 0.0f, 0.0f };

        // Read, process, and output
        for (int channel = 0; channel < numProcessChannels; ++channel)
        {
            float inputSample = buffer.getSample (channel, sample);

            // Calculate modulated delay time
            float lfoValue = lfo[channel].processSample (0.0f);
            float modulatedDelayTime = currentTime + (lfoValue * currentParams.modDepth);
            modulatedDelayTime = juce::jlimit (0.0f, 2.0f, modulatedDelayTime);

            // Read from delay line
            float delayedSample = delayLine[channel].popSample (0, modulatedDelayTime * static_cast<float> (sampleRate));

            // Apply filtering
            float processedSample = lpFilter[channel].processSample (0,
                                    hpFilter[channel].processSample (0, delayedSample));

            // Apply saturation (analog character)
            processedSample = std::tanh (processedSample * currentParams.saturation);

            // Mix dry and wet
            float outSample = (inputSample * (1.0f - currentParams.mix)) + (processedSample * currentParams.mix);
            buffer.setSample (channel, sample, outSample);

            // Prepare feedback signal
            samplesToDelay[channel] = inputSample + (processedSample * currentParams.feedback);
        }

        // Write to delay lines (with ping-pong logic)
        for (int channel = 0; channel < numProcessChannels; ++channel)
        {
            int writeChannel = (currentParams.pingPong && numProcessChannels > 1) ? 1 - channel : channel;
            if (writeChannel >= 2) writeChannel = channel;

            delayLine[writeChannel].pushSample (0, samplesToDelay[channel]);
        }
    }
}

//==============================================================================
EffectModule::ParameterInfo DelayModule::getParameterInfo (int index) const
{
    switch (index)
    {
        case 0: return { "time", "Time", 0.0f, 2.0f, 0.5f, "s", false };
        case 1: return { "feedback", "Feedback", 0.0f, 0.95f, 0.5f, "", false };
        case 2: return { "mix", "Mix", 0.0f, 1.0f, 0.5f, "", false };
        case 3: return { "lpfreq", "LP Freq", 20.0f, 20000.0f, 20000.0f, "Hz", false };
        case 4: return { "hpfreq", "HP Freq", 20.0f, 20000.0f, 20.0f, "Hz", false };
        case 5: return { "pingpong", "Ping Pong", 0.0f, 1.0f, 0.0f, "", true };
        case 6: return { "modrate", "Mod Rate", 0.1f, 10.0f, 1.0f, "Hz", false };
        case 7: return { "moddepth", "Mod Depth", 0.0f, 0.01f, 0.002f, "s", false };
        case 8: return { "saturation", "Saturation", 0.0f, 2.0f, 1.0f, "", false };
        default: return {};
    }
}

void DelayModule::setParameterNormalized (int index, float normalizedValue)
{
    auto info = getParameterInfo (index);
    float value = info.minValue + normalizedValue * (info.maxValue - info.minValue);

    switch (index)
    {
        case 0: currentParams.timeSeconds = value; break;
        case 1: currentParams.feedback = value; break;
        case 2: currentParams.mix = value; break;
        case 3: currentParams.lpFreq = value; break;
        case 4: currentParams.hpFreq = value; break;
        case 5: currentParams.pingPong = normalizedValue > 0.5f; break;
        case 6: currentParams.modRate = value; break;
        case 7: currentParams.modDepth = value; break;
        case 8: currentParams.saturation = value; break;
    }
}

float DelayModule::getParameterNormalized (int index) const
{
    auto info = getParameterInfo (index);
    float value = 0.0f;

    switch (index)
    {
        case 0: value = currentParams.timeSeconds; break;
        case 1: value = currentParams.feedback; break;
        case 2: value = currentParams.mix; break;
        case 3: value = currentParams.lpFreq; break;
        case 4: value = currentParams.hpFreq; break;
        case 5: return currentParams.pingPong ? 1.0f : 0.0f;
        case 6: value = currentParams.modRate; break;
        case 7: value = currentParams.modDepth; break;
        case 8: value = currentParams.saturation; break;
    }

    return (value - info.minValue) / (info.maxValue - info.minValue);
}

juce::ValueTree DelayModule::saveState() const
{
    juce::ValueTree state ("DelayModule");
    state.setProperty ("time", currentParams.timeSeconds, nullptr);
    state.setProperty ("feedback", currentParams.feedback, nullptr);
    state.setProperty ("mix", currentParams.mix, nullptr);
    state.setProperty ("lpfreq", currentParams.lpFreq, nullptr);
    state.setProperty ("hpfreq", currentParams.hpFreq, nullptr);
    state.setProperty ("pingpong", currentParams.pingPong, nullptr);
    state.setProperty ("modrate", currentParams.modRate, nullptr);
    state.setProperty ("moddepth", currentParams.modDepth, nullptr);
    state.setProperty ("saturation", currentParams.saturation, nullptr);
    return state;
}

void DelayModule::loadState (const juce::ValueTree& state)
{
    if (state.hasType ("DelayModule"))
    {
        currentParams.timeSeconds = state.getProperty ("time", 0.5f);
        currentParams.feedback = state.getProperty ("feedback", 0.5f);
        currentParams.mix = state.getProperty ("mix", 0.5f);
        currentParams.lpFreq = state.getProperty ("lpfreq", 20000.0f);
        currentParams.hpFreq = state.getProperty ("hpfreq", 20.0f);
        currentParams.pingPong = state.getProperty ("pingpong", false);
        currentParams.modRate = state.getProperty ("modrate", 1.0f);
        currentParams.modDepth = state.getProperty ("moddepth", 0.002f);
        currentParams.saturation = state.getProperty ("saturation", 1.0f);
    }
}
