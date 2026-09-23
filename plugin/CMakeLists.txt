cmake_minimum_required(VERSION 3.22)
project(VJAnalyzerPlugin VERSION 2.0.0)

# VJ Analyzer - thin VST3 for Ableton Live 10.1+: audio analysis (AnalysisCore),
# performed MIDI, host transport and the 8 macro / preset / blackout controls,
# all sent to the external VJ Engine over OSC protocol v2. It never renders
# visuals itself (REPORT G: keep the renderer out of the audio host).

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VJ_JUCE_PATH "C:/Users/ROI/Documents/harmonizer-vst/JUCE" CACHE PATH "Path to JUCE checkout")
add_subdirectory(${VJ_JUCE_PATH} JUCE_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../analysis analysis_build)

juce_add_plugin(VJAnalyzer
    COMPANY_NAME "VJVST"
    PLUGIN_MANUFACTURER_CODE Vjvs
    PLUGIN_CODE Vjan
    FORMATS VST3
    PRODUCT_NAME "VJ Analyzer"
    VERSION "2.0.0"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Analyzer
)

juce_generate_juce_header(VJAnalyzer)

# Where this checkout builds the engine - the plug-in's "Open Engine" button
# starts it from here unless the user picks another VJ Engine.exe.
get_filename_component(VJ_ENGINE_ABS "${CMAKE_CURRENT_SOURCE_DIR}/../engine/build/VJEngine_artefacts/Release/VJ Engine.exe" ABSOLUTE)
file(TO_NATIVE_PATH "${VJ_ENGINE_ABS}" VJ_ENGINE_EXE)
string(REPLACE "\\" "\\\\" VJ_ENGINE_EXE_ESCAPED "${VJ_ENGINE_EXE}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/EngineLocation.h"
     "#pragma once\n#define VJ_DEFAULT_ENGINE_PATH \"${VJ_ENGINE_EXE_ESCAPED}\"\n")

target_sources(VJAnalyzer PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/AnalysisWorker.cpp
    Source/EngineLauncher.cpp
)
target_include_directories(VJAnalyzer PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_compile_definitions(VJAnalyzer PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(VJAnalyzer PRIVATE
    vj_analysis
    juce::juce_audio_utils
    juce::juce_osc
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Test harness: runs the processor + editor without a DAW (synthetic groove,
# live engine round-trip, PNG of the UI). Not shipped.
juce_add_console_app(PluginHarness PRODUCT_NAME "plugin_harness")
juce_generate_juce_header(PluginHarness)
target_sources(PluginHarness PRIVATE
    tools/PluginHarness.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/AnalysisWorker.cpp
    Source/EngineLauncher.cpp
)
target_include_directories(PluginHarness PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_definitions(PluginHarness PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_MODAL_LOOPS_PERMITTED=1)
target_link_libraries(PluginHarness PRIVATE
    vj_analysis
    juce::juce_audio_utils
    juce::juce_osc
    juce::juce_recommended_config_flags
)
