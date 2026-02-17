# Minimal CMakeLists.txt for unit tests
cmake_minimum_required(VERSION 3.15)
project(VSTFX_Tests)

# Download Catch2 if not present
include(FetchContent)
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(catch2)

add_executable(test_example test_example.cpp)
target_link_libraries(test_example PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(test_example)

# DSP tests
add_executable(dsp_tests dsp_test.cpp dsp_helpers.cpp compressor_math_test.cpp)
target_link_libraries(dsp_tests PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(dsp_tests)
