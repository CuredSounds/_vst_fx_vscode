#include <catch2/catch_all.hpp>
#include "dsp_helpers.h"

TEST_CASE("apply_gain works for positive and negative values", "[dsp]") {
    REQUIRE(dsp::apply_gain(0.5, 2.0) == Catch::Approx(1.0));
    REQUIRE(dsp::apply_gain(-1.0, 0.5) == Catch::Approx(-0.5));
    REQUIRE(dsp::apply_gain(0.0, 10.0) == Catch::Approx(0.0));
}
