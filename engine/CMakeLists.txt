cmake_minimum_required(VERSION 3.22)
project(VJEngine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Reuse the JUCE checkout that already builds cleanly for the harmonizer-vst project
# instead of re-cloning ~1GB. Override with -DVJ_JUCE_PATH=... if you move it.
set(VJ_JUCE_PATH "C:/Users/ROI/Documents/harmonizer-vst/JUCE" CACHE PATH "Path to JUCE checkout")

add_subdirectory(${VJ_JUCE_PATH} JUCE_build)

juce_add_gui_app(VJEngine
    COMPANY_NAME "VJVST"
    BUNDLE_ID "com.vjvst.vjengine"
    PRODUCT_NAME "VJ Engine"
    VERSION "0.1.0"
)

juce_generate_juce_header(VJEngine)

target_sources(VJEngine PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/ISFShader.cpp
    Source/Preset.cpp
    Source/PresetManager.cpp
    Source/SpoutSender.cpp
)

set(VJ_SPOUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Spout")

add_library(SpoutLibrary SHARED IMPORTED)
set_target_properties(SpoutLibrary PROPERTIES
    IMPORTED_IMPLIB "${VJ_SPOUT_DIR}/lib/SpoutLibrary.lib"
    IMPORTED_LOCATION "${VJ_SPOUT_DIR}/bin/SpoutLibrary.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${VJ_SPOUT_DIR}/include"
)

add_custom_command(TARGET VJEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
        "$<TARGET_FILE_DIR:VJEngine>/Shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Presets"
        "$<TARGET_FILE_DIR:VJEngine>/Presets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VJ_SPOUT_DIR}/bin/SpoutLibrary.dll"
        "$<TARGET_FILE_DIR:VJEngine>/SpoutLibrary.dll"
    COMMENT "Copying Shaders/, Presets/, and SpoutLibrary.dll next to the built executable"
)

target_compile_definitions(VJEngine PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VJEngine,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VJEngine,JUCE_VERSION>"
)

target_link_libraries(VJEngine PRIVATE
    juce::juce_gui_extra
    juce::juce_opengl
    juce::juce_osc
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    SpoutLibrary
)
