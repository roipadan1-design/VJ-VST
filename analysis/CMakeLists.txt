cmake_minimum_required(VERSION 3.22)
project(VJAnalysis VERSION 2.0.0 LANGUAGES CXX)

# AnalysisCore: host-independent audio analysis + the v2 wire protocol.
# No JUCE, no Max, no GL - the VJ Analyzer plugin, the test suite and the
# command-line tools all link this same library.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(vj_analysis STATIC
    src/Fft.cpp
    src/AdaptiveNormalizer.cpp
    src/OnsetTracker.cpp
    src/Analyzer.cpp
    src/Protocol.cpp
)
target_include_directories(vj_analysis PUBLIC include)
set_target_properties(vj_analysis PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    target_compile_options(vj_analysis PRIVATE /W4 /permissive-)
endif()

# Tests and tools only when this is the top-level project (the plugin build
# pulls the library in via add_subdirectory and doesn't need them).
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_executable(analysis_tests tests/analysis_tests.cpp)
    target_link_libraries(analysis_tests PRIVATE vj_analysis)

    add_executable(analyze_wav tools/analyze_wav.cpp)
    target_link_libraries(analyze_wav PRIVATE vj_analysis ws2_32)

    enable_testing()
    add_test(NAME analysis_tests COMMAND analysis_tests)
endif()
