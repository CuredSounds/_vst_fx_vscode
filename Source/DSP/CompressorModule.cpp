#include "CompressorModule.h"
#include "CompressorMath.h"

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

    // Sidechain scratch buffer
    sidechainScratchBuffer.setSize (numChannels, maxBlockSize);

    // Prepare sidechain filter
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<uint32_t> (maxBlockSize);
    spec.numChannels = static_cast<uint32_t> (numChannels);
    sidechainFilter.reset();
    sidechainFilter.prepare (spec);

    // Prepare lookahead buffer (maxLookaheadMs + block margin)
    lookaheadBufferSize = static_cast<int> (std::ceil ((maxLookaheadMs / 1000.0) * sampleRate)) + maxBlockSize + 8;
    lookaheadBuffer.setSize (numChannels, lookaheadBufferSize);
    lookaheadBuffer.clear();
    lookaheadWritePos = 0;
    lookaheadDelaySamples = 0;

    // Remember max block size for potential oversampler initialization
    this->maxBlockSize = maxBlockSize;

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

    // Update lookahead delay (in samples) from parameters
    if (params.lookaheadEnabled)
    {
        const int requested = static_cast<int> (std::round (params.lookaheadMs * sampleRate / 1000.0));
        lookaheadDelaySamples = juce::jlimit (0, lookaheadBufferSize - 1, requested);
    }
    else
    {
        lookaheadDelaySamples = 0;
    }

    // --- Oversampling: lazy init of juce::dsp::Oversampling when requested ---
    if (params.oversampleEnabled && params.oversampleFactor > 1)
    {
        const int requestedFactor = params.oversampleFactor;
        if (! oversampler || static_cast<int> (oversampler->getOversamplingFactor()) != requestedFactor)
        {
            // compute exponent (factor = 2^exp)
            unsigned int exp = 0;
            int tmp = requestedFactor;
            while (tmp > 1) { tmp >>= 1; ++exp; }

            oversampler.reset (new juce::dsp::Oversampling<float> (static_cast<size_t> (numChannels), exp, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false));
            oversampler->initProcessing (static_cast<size_t> (maxBlockSize));
        }
    }
    else
    {
        oversampler.reset();
    }

    // Store dry signal for parallel compression (for no-lookahead case we copy upfront)
    const bool needsDrySignal = params.mixPercent < 100.0f;
    if (needsDrySignal && params.lookaheadEnabled == false)
    {
        for (int ch = 0; ch < numProcessChannels; ++ch)
            dryBuffer.copyFrom (ch, 0, mainBuffer, ch, 0, numSamples);
    }

    // Determine detection source (main or sidechain)
    const auto* detectionBuffer = params.useSidechain && sidechainBuffer != nullptr
                                  ? sidechainBuffer
                                  : &mainBuffer;

    // If sidechain HPF is requested and a sidechain buffer is present, copy and filter into scratch
    const juce::AudioBuffer<float>* filteredDetection = detectionBuffer;
    if (params.useSidechain && sidechainBuffer != nullptr && params.sidechainHpfFreq > 20.0f)
    {

    // Copy into scratch
    sidechainScratchBuffer.makeCopyOf (*sidechainBuffer, true);

    // Build audio block from scratch buffer and process filter
    juce::dsp::AudioBlock<float> block (const_cast<float**> (sidechainScratchBuffer.getArrayOfWritePointers()),
                        static_cast<size_t> (sidechainScratchBuffer.getNumChannels()),
                        static_cast<size_t> (sidechainScratchBuffer.getNumSamples()));

    // Set new coefficients for high-pass (using public coefficients method)
    sidechainFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, params.sidechainHpfFreq, 0.7071f);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    sidechainFilter.process (ctx);

        filteredDetection = &sidechainScratchBuffer;
    }

    const int detectionChannels = std::min (numProcessChannels, filteredDetection->getNumChannels());

    // Peak GR tracker for metering
    float peakGR = 0.0f;
    
    // Level metering
    float peakInputLevel = 0.0f;
    float peakOutputLevel = 0.0f;

    // Process each sample (supports optional lookahead)
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 1. Compute detection signal (peak across channels)
        float detectionLevel = 0.0f;
        for (int ch = 0; ch < detectionChannels; ++ch)
        {
            const float inputSample = filteredDetection->getReadPointer (ch)[sample];
            detectionLevel = std::max (detectionLevel, std::abs (inputSample));
        }
        
        // Track input level (from main buffer)
        for (int ch = 0; ch < numProcessChannels; ++ch)
        {
            const float sample_val = mainBuffer.getReadPointer (ch)[sample];
            peakInputLevel = std::max (peakInputLevel, std::abs (sample_val));
        }

        // 2. Apply envelope follower (per-channel ballistics)
        float& envelope = envelopeState[0]; // Stereo-linked

        if (detectionLevel > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * detectionLevel;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * detectionLevel;

        // 3. Convert to dB
        const float envelopeDb = juce::Decibels::gainToDecibels (envelope + 1e-6f); // Avoid log(0)

        // 4. Compute gain reduction using header-only math helpers
        const float currentThreshold = smoothedThreshold.getNextValue();
        const float currentRatio = smoothedRatio.getNextValue();

        const float gainReductionDb = bc_dsp::computeGainReduction (envelopeDb, currentThreshold, currentRatio, params.kneeDb);

        // Track peak GR for metering
        peakGR = std::min (peakGR, gainReductionDb); // GR is negative

        // 5. Convert to linear gain and apply makeup
        const float makeupGain = juce::Decibels::decibelsToGain (smoothedMakeup.getNextValue());
        const float totalGain = juce::Decibels::decibelsToGain (gainReductionDb) * makeupGain;

        if (params.lookaheadEnabled && params.lookaheadMs > 0.001f)
        {
            // Write current input samples into lookahead buffer at write position
            for (int ch = 0; ch < numProcessChannels; ++ch)
            {
                const float inS = mainBuffer.getReadPointer (ch)[sample];
                lookaheadBuffer.setSample (ch, lookaheadWritePos, inS);
            }

            // Compute read position for delayed output
            const int readPos = (lookaheadWritePos + lookaheadBufferSize - lookaheadDelaySamples) % lookaheadBufferSize;

            // Read delayed samples, apply gain, and write back to main buffer
            for (int ch = 0; ch < numProcessChannels; ++ch)
            {
                const float delayed = lookaheadBuffer.getSample (ch, readPos);
                float processed = delayed * totalGain;
                mainBuffer.getWritePointer (ch)[sample] = processed;
                
                // Track output level
                peakOutputLevel = std::max (peakOutputLevel, std::abs (processed));

                if (needsDrySignal)
                    dryBuffer.setSample (ch, sample, delayed);
            }

            // advance write position
            lookaheadWritePos = (lookaheadWritePos + 1) % lookaheadBufferSize;
        }
        else
        {
            // No lookahead: apply gain directly to current buffer
            for (int ch = 0; ch < numProcessChannels; ++ch)
            {
                float* channelData = mainBuffer.getWritePointer (ch);
                channelData[sample] *= totalGain;
                
                // Track output level
                peakOutputLevel = std::max (peakOutputLevel, std::abs (channelData[sample]));
            }
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
            {
                const float mixed = wetData[sample] * wetGain + dryData[sample] * dryGain;
                wetData[sample] = mixed;
                peakOutputLevel = std::max (peakOutputLevel, std::abs (mixed));
            }
        }
    }

    // Store metrics for metering
    currentGainReductionDb.store (peakGR);
    currentInputLevelDb.store (peakInputLevel > 0.0f ? juce::Decibels::gainToDecibels (peakInputLevel) : -80.0f);
    currentOutputLevelDb.store (peakOutputLevel > 0.0f ? juce::Decibels::gainToDecibels (peakOutputLevel) : -80.0f);
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
        case 8: return { "sc_hpf", "Sidechain HPF", 20.0f, 2000.0f, 20.0f, "Hz", false };
    case 9: return { "lookahead", "Lookahead", 0.0f, 1.0f, 0.0f, "", true };
    case 10: return { "lookahead_ms", "Lookahead (ms)", 0.0f, 100.0f, 5.0f, "ms", false };
    case 11: return { "oversample", "Oversample", 0.0f, 1.0f, 0.0f, "", true };
    case 12: return { "oversample_factor", "Oversample Factor", 1.0f, 4.0f, 1.0f, "x", false };
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
        case 8: currentParams.sidechainHpfFreq = value; break;
        case 9: currentParams.lookaheadEnabled = normalizedValue > 0.5f; break;
    case 10: currentParams.lookaheadMs = value; break;
    case 11: currentParams.oversampleEnabled = normalizedValue > 0.5f; break;
    case 12: currentParams.oversampleFactor = static_cast<int> (value) == 2 ? 2 : (static_cast<int> (value) == 4 ? 4 : 1); break;
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
        case 8: return currentParams.sidechainHpfFreq;
        case 9: return currentParams.lookaheadEnabled ? 1.0f : 0.0f;
    case 10: return currentParams.lookaheadMs;
    case 11: return currentParams.oversampleEnabled ? 1.0f : 0.0f;
    case 12: return static_cast<float> (currentParams.oversampleFactor);
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
    state.setProperty ("sc_hpf", currentParams.sidechainHpfFreq, nullptr);
    state.setProperty ("lookahead", currentParams.lookaheadEnabled, nullptr);
    state.setProperty ("lookahead_ms", currentParams.lookaheadMs, nullptr);
    state.setProperty ("oversample", currentParams.oversampleEnabled, nullptr);
    state.setProperty ("oversample_factor", currentParams.oversampleFactor, nullptr);
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
        currentParams.sidechainHpfFreq = state.getProperty ("sc_hpf", 20.0f);
        currentParams.lookaheadEnabled = state.getProperty ("lookahead", false);
        currentParams.lookaheadMs = state.getProperty ("lookahead_ms", 5.0f);
        currentParams.oversampleEnabled = state.getProperty ("oversample", false);
        currentParams.oversampleFactor = static_cast<int> (state.getProperty ("oversample_factor", 1));
    }
}

EffectModule::MeterData CompressorModule::getMeterData() const
{
    MeterData data;
    data.inputLevel = currentInputLevelDb.load();
    data.outputLevel = currentOutputLevelDb.load();
    data.gainReduction = currentGainReductionDb.load();
    return data;
}
