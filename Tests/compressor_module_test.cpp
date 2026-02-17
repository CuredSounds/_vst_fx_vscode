// Placeholder test: JUCE-dependent tests require adding JUCE include paths and
// CMake configuration for generating JuceHeader.h in the Tests target. Keep
// this placeholder so `dsp_tests` builds on CI; replace with a real
// CompressorModule test after configuring JUCE for test builds.

#include <catch2/catch_all.hpp>

TEST_CASE("placeholder - compressor module test scaffold", "[compressor_module]")
{
    REQUIRE( true );
}
