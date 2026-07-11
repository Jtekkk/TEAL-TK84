cmake_minimum_required(VERSION 3.22)

project(TEAL_TK84 VERSION 1.0.0 LANGUAGES C CXX)

# ----------------------------------------------------------------------------
# JUCE acquisition
#
# By default the build fetches JUCE 8 automatically so it works out of the box.
# To use a local JUCE checkout instead (faster incremental builds), configure
# with -DJUCE_DIR=/path/to/JUCE.
# ----------------------------------------------------------------------------
set(JUCE_GIT_TAG "8.0.4" CACHE STRING "JUCE git tag/branch to fetch when JUCE_DIR is not provided")

if(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(STATUS "TEAL TK84: using local JUCE at ${JUCE_DIR}")
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    message(STATUS "TEAL TK84: JUCE_DIR not set — fetching JUCE ${JUCE_GIT_TAG} via FetchContent")
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        ${JUCE_GIT_TAG}
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ----------------------------------------------------------------------------
# Plugin formats
#
#   VST3 + Standalone  : built everywhere
#   AU                 : Apple platforms only
#   AAX                : opt-in, requires the proprietary AAX SDK
#                        configure with -DTK84_BUILD_AAX=ON -DAAX_SDK_PATH=/path
# ----------------------------------------------------------------------------
set(TK84_FORMATS VST3 Standalone)

if(APPLE)
    list(APPEND TK84_FORMATS AU)
endif()

option(TK84_BUILD_AAX "Build the AAX plugin format (requires the AAX SDK)" OFF)
if(TK84_BUILD_AAX)
    if(NOT DEFINED AAX_SDK_PATH OR NOT EXISTS "${AAX_SDK_PATH}")
        message(FATAL_ERROR "TK84_BUILD_AAX is ON but AAX_SDK_PATH is not set to a valid AAX SDK path")
    endif()
    juce_set_aax_sdk_path(${AAX_SDK_PATH})
    list(APPEND TK84_FORMATS AAX)
endif()

message(STATUS "TEAL TK84: building formats: ${TK84_FORMATS}")

juce_add_plugin(TEAL_TK84
    COMPANY_NAME "TEAL"
    BUNDLE_ID "com.teal.tk84"
    PLUGIN_MANUFACTURER_CODE "Teal"
    PLUGIN_CODE "TK84"
    FORMATS ${TK84_FORMATS}
    PRODUCT_NAME "TEAL TK84"
    DESCRIPTION "Stereo VCA Bus Compressor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Sources include <JuceHeader.h>, so it must be generated for this target.
juce_generate_juce_header(TEAL_TK84)

target_sources(TEAL_TK84 PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    TK84Engine.cpp
    TK84Detector.cpp
    TK84GainComputer.cpp
    TK84VCA.cpp
    TK84Transient.cpp
    SteppedRotarySlider.cpp
    GRLEDMeter.cpp
    IlluminatedVUMeter.cpp
    TK84LookAndFeel.cpp
)

target_compile_features(TEAL_TK84 PRIVATE cxx_std_17)

target_include_directories(TEAL_TK84 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(TEAL_TK84 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(TEAL_TK84
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
