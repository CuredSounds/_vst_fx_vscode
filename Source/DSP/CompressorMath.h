#pragma once

#include <cmath>

namespace bc_dsp
{
    // Compute gain reduction in dB for given input level (dB), threshold, ratio and knee.
    inline float computeGainReduction(float inputDb, float thresholdDb, float ratio, float kneeDb)
    {
        const float overdB = inputDb - thresholdDb;
        float gainReductionDb = 0.0f;

        if (kneeDb <= 0.01f)
        {
            if (overdB > 0.0f)
                gainReductionDb = overdB * (1.0f - 1.0f / ratio);
        }
        else
        {
            const float kneeHalf = kneeDb / 2.0f;

            if (overdB < -kneeHalf)
            {
                gainReductionDb = 0.0f;
            }
            else if (overdB > kneeHalf)
            {
                gainReductionDb = overdB * (1.0f - 1.0f / ratio);
            }
            else
            {
                const float x = overdB + kneeHalf;
                gainReductionDb = (x * x) / (2.0f * kneeDb) * (1.0f - 1.0f / ratio);
            }
        }

        return -gainReductionDb; // Gain reduction is negative
    }

    inline float attackCoeffFromMs(float attackMs, double sampleRate)
    {
        return std::exp(-1.0f / (attackMs * 0.001f * static_cast<float>(sampleRate)));
    }

    inline float releaseCoeffFromMs(float releaseMs, double sampleRate)
    {
        return std::exp(-1.0f / (releaseMs * 0.001f * static_cast<float>(sampleRate)));
    }

    inline int msToSamples(float ms, double sampleRate)
    {
        return static_cast<int>(std::round(ms * sampleRate / 1000.0));
    }
}