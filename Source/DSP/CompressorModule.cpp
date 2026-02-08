#include "CompressorModule.h"

//==============================================================================
CompressorModule::CompressorModule()
{
    // Initialize parameters to defaults
    currentParams.thresholdDb = -20.0f;
    currentParams.ratio = 4.0f;
    currentParams.attackMs = 5.0f;
    currentParams.releaseMs = 100.0f;
    currentParams.kneeDb = 0.0f;
    currentParams.makeupDb = 0.0f;
    currentParams.mixPercent = 100.0f;
    currentParams.useSidechain = false;
}

//==============================================================================
void CompressorModule::prepare (double newSampleRate, int maxBlockSize, int newNumChannels)
{
    sampleRate = newSampleRate;
    numChannels = newNumChannels;

    // Allocate envelope state per channel
    envelopeState.resize (numChannels, 0.0f);

    // Allocate dry buffer for parallel compression
    dryBuffer.setSize (numChannels, maxBlockSize);

    // Set smoothing ramp (20ms for parameter changes)
    const double smoothingTimeMs = 20.0;
    const int smoothingSamples = static_cast<int> (sampleRate * smoothingTimeMs / 1000.0);

    smoothedThreshold.reset (sampleRate, smoothingTimeMs / 1000.0);
    smoothedRatio.reset (sampleRate, smoothingTimeMs / 1000.0);
    smoothedMakeup.reset (sampleRate, smoothingTimeMs / 1000.0);
    smoothedMix.reset (sampleRate, smoothingTimeMs / 1000.0);

    reset();
}

void CompressorModule::reset()
{
    std::fill (envelopeState.begin(), envelopeState.end(), 0.0f);
    currentGainReductionDb.store (0.0f);
    dryBuffer.clear();
}

//==============================================================================
void CompressorModule::process (juce::AudioBuffer<float>& mainBuffer,
                                 const juce::AudioBuffer<float>* sidechainBuffer,
                                 const Parameters& params)
{
    const int numSamples = mainBuffer.getNumSamples();
    const int numProcessChannels = std::min (numChannels, mainBuffer.getNumChannels());

    if (numSamples == 0 || numProcessChannels == 0)
        return;

    // Update smoothed parameters
    updateSmoothers (params);

    // Compute attack/release coefficients from ms values
    // Using exponential smoothing: coeff = exp(-1 / (time_in_seconds * sample_rate))
    attackCoeff = std::exp (-1.0f / (params.attackMs * 0.001f * static_cast<float> (sampleRate)));
    releaseCoeff = std::exp (-1.0f / (params.releaseMs * 0.001f * static_cast<float> (sampleRate)));

    // Store dry signal for parallel compression
    const bool needsDrySignal = params.mixPercent < 100.0f;
    if (needsDrySignal)
    {
        for (int ch = 0; ch < numProcessChannels; ++ch)
            dryBuffer.copyFrom (ch, 0, mainBuffer, ch, 0, numSamples);
    }

    // Determine detection source (main or sidechain)
    const auto* detectionBuffer = params.useSidechain && sidechainBuffer != nullptr
                                  ? sidechainBuffer
                                  : &mainBuffer;

    const int detectionChannels = std::min (numProcessChannels, detectionBuffer->getNumChannels());

    // Peak GR tracker for metering
    float peakGR = 0.0f;

    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 1. Compute detection signal (peak across channels)
        float detectionLevel = 0.0f;
        for (int ch = 0; ch < detectionChannels; ++ch)
        {
            const float inputSample = detectionBuffer->getReadPointer (ch)[sample];
            detectionLevel = std::max (detectionLevel, std::abs (inputSample));
        }

        // 2. Apply envelope follower (per-channel ballistics)
        // For stereo link, we use the same envelope across channels
        float& envelope = envelopeState[0]; // Stereo-linked

        if (detectionLevel > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * detectionLevel;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * detectionLevel;

        // 3. Convert to dB
        const float envelopeDb = juce::Decibels::gainToDecibels (envelope + 1e-6f); // Avoid log(0)

        // 4. Compute gain reduction
        const float currentThreshold = smoothedThreshold.getNextValue();
        const float currentRatio = smoothedRatio.getNextValue();

        Parameters currentParams = params;
        currentParams.thresholdDb = currentThreshold;
        currentParams.ratio = currentRatio;

        const float gainReductionDb = computeGainReduction (envelopeDb, currentParams);

        // Track peak GR for metering
        peakGR = std::min (peakGR, gainReductionDb); // GR is negative

        // 5. Convert to linear gain and apply makeup
        const float makeupGain = juce::Decibels::decibelsToGain (smoothedMakeup.getNextValue());
        const float totalGain = juce::Decibels::decibelsToGain (gainReductionDb) * makeupGain;

        // 6. Apply gain to all output channels
        for (int ch = 0; ch < numProcessChannels; ++ch)
        {
            float* channelData = mainBuffer.getWritePointer (ch);
            channelData[sample] *= totalGain;
        }
    }

    // 7. Apply parallel compression (dry/wet mix)
    if (needsDrySignal)
    {
        const float wetGain = smoothedMix.getCurrentValue() / 100.0f;
        const float dryGain = 1.0f - wetGain;

        for (int ch = 0; ch < numProcessChannels; ++ch)
        {
            float* wetData = mainBuffer.getWritePointer (ch);
            const float* dryData = dryBuffer.getReadPointer (ch);

            for (int sample = 0; sample < numSamples; ++sample)
                wetData[sample] = wetData[sample] * wetGain + dryData[sample] * dryGain;
        }
    }

    // Store GR for metering
    currentGainReductionDb.store (peakGR);
}

//==============================================================================
float CompressorModule::computeGainReduction (float inputDb, const Parameters& params) const
{
    // Standard compressor gain computer with soft knee

    const float threshold = params.thresholdDb;
    const float ratio = params.ratio;
    const float knee = params.kneeDb;

    float gainReductionDb = 0.0f;

    // Amount over threshold
    const float overdB = inputDb - threshold;

    // Hard knee
    if (knee <= 0.01f)
    {
        if (overdB > 0.0f)
            gainReductionDb = overdB * (1.0f - 1.0f / ratio);
    }
    // Soft knee
    else
    {
        const float kneeHalf = knee / 2.0f;

        if (overdB < -kneeHalf)
        {
            // Below knee range - no compression
            gainReductionDb = 0.0f;
        }
        else if (overdB > kneeHalf)
        {
            // Above knee range - full compression
            gainReductionDb = overdB * (1.0f - 1.0f / ratio);
        }
        else
        {
            // Inside knee range - smooth transition
            const float x = overdB + kneeHalf;
            gainReductionDb = (x * x) / (2.0f * knee) * (1.0f - 1.0f / ratio);
        }
    }

    // Gain reduction is always negative
    return -gainReductionDb;
}

void CompressorModule::updateSmoothers (const Parameters& params)
{
    smoothedThreshold.setTargetValue (params.thresholdDb);
    smoothedRatio.setTargetValue (params.ratio);
    smoothedMakeup.setTargetValue (params.makeupDb);
    smoothedMix.setTargetValue (params.mixPercent);
}

//==============================================================================
// EffectModule interface implementation

void CompressorModule::process (juce::AudioBuffer<float>& buffer,
                                 const juce::AudioBuffer<float>* sidechainBuffer)
{
    // Call the legacy process method with currentParams
    process (buffer, sidechainBuffer, currentParams);
}

EffectModule::ParameterInfo CompressorModule::getParameterInfo (int index) const
{
    switch (index)
    {
        case 0: return { "threshold", "Threshold", -60.0f, 0.0f, -20.0f, "dB", false };
        case 1: return { "ratio", "Ratio", 1.0f, 20.0f, 4.0f, "", false };
        case 2: return { "attack", "Attack", 0.1f, 100.0f, 5.0f, "ms", false };
        case 3: return { "release", "Release", 10.0f, 1000.0f, 100.0f, "ms", false };
        case 4: return { "knee", "Knee", 0.0f, 12.0f, 0.0f, "dB", false };
        case 5: return { "makeup", "Makeup Gain", -12.0f, 12.0f, 0.0f, "dB", false };
        case 6: return { "mix", "Mix", 0.0f, 100.0f, 100.0f, "%", false };
        case 7: return { "sidechain", "Sidechain", 0.0f, 1.0f, 0.0f, "", true };
        default: return {};
    }
}

void CompressorModule::setParameterNormalized (int index, float normalizedValue)
{
    auto info = getParameterInfo (index);
    float value = info.minValue + normalizedValue * (info.maxValue - info.minValue);

    switch (index)
    {
        case 0: currentParams.thresholdDb = value; break;
        case 1: currentParams.ratio = value; break;
        case 2: currentParams.attackMs = value; break;
        case 3: currentParams.releaseMs = value; break;
        case 4: currentParams.kneeDb = value; break;
        case 5: currentParams.makeupDb = value; break;
        case 6: currentParams.mixPercent = value; break;
        case 7: currentParams.useSidechain = normalizedValue > 0.5f; break;
    }
}

float CompressorModule::getParameterNormalized (int index) const
{
    auto info = getParameterInfo (index);
    float value = 0.0f;

    switch (index)
    {
        case 0: value = currentParams.thresholdDb; break;
        case 1: value = currentParams.ratio; break;
        case 2: value = currentParams.attackMs; break;
        case 3: value = currentParams.releaseMs; break;
        case 4: value = currentParams.kneeDb; break;
        case 5: value = currentParams.makeupDb; break;
        case 6: value = currentParams.mixPercent; break;
        case 7: return currentParams.useSidechain ? 1.0f : 0.0f;
    }

    return (value - info.minValue) / (info.maxValue - info.minValue);
}

juce::ValueTree CompressorModule::saveState() const
{
    juce::ValueTree state ("CompressorModule");
    state.setProperty ("threshold", currentParams.thresholdDb, nullptr);
    state.setProperty ("ratio", currentParams.ratio, nullptr);
    state.setProperty ("attack", currentParams.attackMs, nullptr);
    state.setProperty ("release", currentParams.releaseMs, nullptr);
    state.setProperty ("knee", currentParams.kneeDb, nullptr);
    state.setProperty ("makeup", currentParams.makeupDb, nullptr);
    state.setProperty ("mix", currentParams.mixPercent, nullptr);
    state.setProperty ("sidechain", currentParams.useSidechain, nullptr);
    return state;
}

void CompressorModule::loadState (const juce::ValueTree& state)
{
    if (state.hasType ("CompressorModule"))
    {
        currentParams.thresholdDb = state.getProperty ("threshold", -20.0f);
        currentParams.ratio = state.getProperty ("ratio", 4.0f);
        currentParams.attackMs = state.getProperty ("attack", 5.0f);
        currentParams.releaseMs = state.getProperty ("release", 100.0f);
        currentParams.kneeDb = state.getProperty ("knee", 0.0f);
        currentParams.makeupDb = state.getProperty ("makeup", 0.0f);
        currentParams.mixPercent = state.getProperty ("mix", 100.0f);
        currentParams.useSidechain = state.getProperty ("sidechain", false);
    }
}

EffectModule::MeterData CompressorModule::getMeterData() const
{
    MeterData data;
    data.gainReduction = currentGainReductionDb.load();
    return data;
}
