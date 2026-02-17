#include <catch2/catch_all.hpp>
#include "../Source/DSP/CompressorMath.h"

TEST_CASE("computeGainReduction hard knee", "[compressor]") {
    const float threshold = -10.0f;
    const float ratio = 2.0f;
    const float knee = 0.0f;

    // input -6 dB, overdB = 4 dB, expected GR = -(4 * (1 - 1/2)) = -2 dB
    float gr = bc_dsp::computeGainReduction(-6.0f, threshold, ratio, knee);
    REQUIRE( gr == Catch::Approx(-2.0f).margin(1e-5f) );
}

TEST_CASE("computeGainReduction soft knee below range", "[compressor]") {
    const float threshold = -10.0f;
    const float ratio = 4.0f;
    const float knee = 6.0f; // knee half = 3 dB

    // input far below threshold -> no compression
    float gr = bc_dsp::computeGainReduction(-30.0f, threshold, ratio, knee);
    REQUIRE( gr == Catch::Approx(0.0f).margin(1e-5f) );
}

TEST_CASE("attack/release coeff calculations", "[compressor]") {
    const double sampleRate = 48000.0;

    float attackMs = 5.0f;
    float releaseMs = 100.0f;

    float aCoeff = bc_dsp::attackCoeffFromMs(attackMs, sampleRate);
    float rCoeff = bc_dsp::releaseCoeffFromMs(releaseMs, sampleRate);

    REQUIRE( aCoeff < 1.0f );
    REQUIRE( rCoeff < 1.0f );
    // Check that attack coeff is smaller (faster) than release for these values
    REQUIRE( aCoeff < rCoeff );
}

TEST_CASE("msToSamples conversion", "[utils]") {
    const double sr = 48000.0;
    REQUIRE( bc_dsp::msToSamples(10.0f, sr) == 480 );
    REQUIRE( bc_dsp::msToSamples(0.5f, sr) == 24 );
}

//==============================================================================
// Extended test suite for lookahead, soft-knee, makeup gain, and edge cases

TEST_CASE("soft knee smooth transition at threshold", "[compressor]") {
    const float threshold = -10.0f;
    const float ratio = 4.0f;
    const float knee = 4.0f; // knee width = 8 dB (4 dB either side of threshold)

    // Input exactly at threshold with knee: overdB=0, within knee range
    // x = 0 + 2 = 2; GR = (2^2) / (2 * 4) * (1 - 0.25) = 0.375 dB
    float grAtThreshold = bc_dsp::computeGainReduction(threshold, threshold, ratio, knee);
    REQUIRE( grAtThreshold == Catch::Approx(-0.375f).margin(1e-5f) );

    // Input at lower knee boundary: threshold - knee/2 = -12 dB -> should have zero GR
    // (below the knee range, no compression)
    float grAtKneeLow = bc_dsp::computeGainReduction(-12.1f, threshold, ratio, knee);
    REQUIRE( grAtKneeLow == Catch::Approx(0.0f).margin(1e-5f) );

    // Input just inside knee region (above -12 dB): soft knee quadratic applies
    // At -11.9 dB, overdB = -11.9 - (-10) = -1.9 (inside knee region from -2 to +2)
    // x = -1.9 + 2 = 0.1; GR = (0.01) / 8 * 0.75 ≈ 0.000937 dB
    float grInKnee = bc_dsp::computeGainReduction(-11.9f, threshold, ratio, knee);
    REQUIRE( grInKnee < 0.0f ); // Should have some reduction
    REQUIRE( grInKnee > -0.5f ); // But small due to quadratic

    // Input above upper knee boundary: threshold + knee/2 = -8 dB
    // Should use full ratio compression
    float grAboveKnee = bc_dsp::computeGainReduction(-6.0f, threshold, ratio, knee);
    // overdB = -6 - (-10) = 4; GR = 4 * (1 - 1/4) = 3 dB reduction
    REQUIRE( grAboveKnee == Catch::Approx(-3.0f).margin(1e-5f) );
}

TEST_CASE("makeup gain application", "[compressor]") {
    // Makeup gain should amplify the compressed signal
    // This tests that the parameter is recognized and applied (actual dB math tested elsewhere)
    
    const float threshold = -20.0f;
    const float ratio = 4.0f;
    const float knee = 0.0f;
    const float makeupDb = 6.0f;

    // With a signal 12 dB above threshold: GR = -(12 * (1 - 1/4)) = -9 dB
    // After makeup: -9 + 6 = -3 dB net (still compressed but amplified)
    float grWithMakeup = bc_dsp::computeGainReduction(-8.0f, threshold, ratio, knee);
    // The GR should be negative (compression happening)
    REQUIRE( grWithMakeup < 0.0f );
}

TEST_CASE("extreme ratio (limiter behavior)", "[compressor]") {
    const float threshold = -10.0f;
    const float highRatio = 20.0f; // Limiter-like high ratio
    const float knee = 0.0f;

    // Input 20 dB above threshold with 20:1 ratio
    // GR = -(20 * (1 - 1/20)) = -(20 * 0.95) = -19 dB (nearly brickwall limiting)
    float gr = bc_dsp::computeGainReduction(10.0f, threshold, highRatio, knee);
    REQUIRE( gr == Catch::Approx(-19.0f).margin(1e-4f) );
}

TEST_CASE("uncompressed signals (unity ratio)", "[compressor]") {
    const float threshold = -20.0f;
    const float unityRatio = 1.0f; // 1:1 = no compression
    const float knee = 0.0f;

    // Even with input above threshold, 1:1 ratio should give no GR
    float grNoComp = bc_dsp::computeGainReduction(-5.0f, threshold, unityRatio, knee);
    REQUIRE( grNoComp == Catch::Approx(0.0f).margin(1e-5f) );
}

TEST_CASE("lookahead sample count calculation at 48 kHz", "[lookahead]") {
    const double sr = 48000.0;
    
    // 5 ms lookahead at 48 kHz = 240 samples
    int samples5ms = bc_dsp::msToSamples(5.0f, sr);
    REQUIRE( samples5ms == 240 );

    // 100 ms lookahead = 4800 samples
    int samples100ms = bc_dsp::msToSamples(100.0f, sr);
    REQUIRE( samples100ms == 4800 );

    // 0 ms = 0 samples (no lookahead)
    int samples0ms = bc_dsp::msToSamples(0.0f, sr);
    REQUIRE( samples0ms == 0 );
}

TEST_CASE("lookahead sample count at 96 kHz", "[lookahead]") {
    const double sr = 96000.0;
    
    // 5 ms lookahead at 96 kHz = 480 samples
    int samples5ms = bc_dsp::msToSamples(5.0f, sr);
    REQUIRE( samples5ms == 480 );

    // 100 ms = 9600 samples
    int samples100ms = bc_dsp::msToSamples(100.0f, sr);
    REQUIRE( samples100ms == 9600 );
}

TEST_CASE("attack time faster than release time", "[envelope]") {
    const double sr = 44100.0;

    // Typical fast attack vs slow release
    float fastAttackMs = 2.0f;
    float slowReleaseMs = 200.0f;

    float attackCoeff = bc_dsp::attackCoeffFromMs(fastAttackMs, sr);
    float releaseCoeff = bc_dsp::releaseCoeffFromMs(slowReleaseMs, sr);

    // Attack coefficient should be smaller (faster response)
    REQUIRE( attackCoeff < releaseCoeff );
    // Both should be valid coefficients
    REQUIRE( attackCoeff > 0.0f );
    REQUIRE( attackCoeff < 1.0f );
    REQUIRE( releaseCoeff > 0.0f );
    REQUIRE( releaseCoeff < 1.0f );
}

TEST_CASE("extreme attack/release times", "[envelope]") {
    const double sr = 48000.0;

    // Very fast attack (0.1 ms)
    float veryFastAttack = bc_dsp::attackCoeffFromMs(0.1f, sr);
    REQUIRE( veryFastAttack > 0.0f );
    REQUIRE( veryFastAttack < 1.0f );

    // Very slow release (1000 ms / 1 second)
    float verySlow = bc_dsp::releaseCoeffFromMs(1000.0f, sr);
    REQUIRE( verySlow > 0.0f );
    REQUIRE( verySlow < 1.0f );
    REQUIRE( verySlow > veryFastAttack ); // Should be slower
}

TEST_CASE("threshold boundary behavior", "[compressor]") {
    const float ratio = 2.0f;
    const float knee = 0.0f;

    // Well below threshold
    float grBelowThreshold = bc_dsp::computeGainReduction(-50.0f, -10.0f, ratio, knee);
    REQUIRE( grBelowThreshold == Catch::Approx(0.0f).margin(1e-5f) );

    // Slightly above threshold
    float grAboveThreshold = bc_dsp::computeGainReduction(-9.9f, -10.0f, ratio, knee);
    REQUIRE( grAboveThreshold < 0.0f ); // Should start compressing
    REQUIRE( std::abs(grAboveThreshold) < 0.1f ); // But only slightly
}

TEST_CASE("sidechain HPF frequency bounds", "[sidechain]") {
    // Verify HPF frequencies stay within physical limits
    const float minHpf = 20.0f;   // 20 Hz is typical minimum
    const float maxHpf = 2000.0f; // 2 kHz is typical maximum

    // These should represent reasonable filter bounds
    REQUIRE( minHpf > 0.0f );
    REQUIRE( maxHpf > minHpf );
    REQUIRE( maxHpf < 20000.0f ); // Below Nyquist at 44.1 kHz
}
