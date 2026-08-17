cmake_minimum_required(VERSION 3.22)

project(ChordFlowMelody VERSION 1.0.0)

if(NOT DEFINED JUCE_DIR)
    message(FATAL_ERROR "JUCE_DIR has not been specified")
endif()

add_subdirectory(
    "${JUCE_DIR}"
    "${CMAKE_BINARY_DIR}/JUCE"
)

juce_add_plugin(ChordFlowMelody
    COMPANY_NAME "Good Aura Music"
    PRODUCT_NAME "ChordFlow Melody"
    PLUGIN_MANUFACTURER_CODE Gaur
    PLUGIN_CODE CfMl

    FORMATS VST3

    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE

    VST3_CATEGORIES "Generator"

    JUCE_VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(ChordFlowMelody
    PRIVATE
        PluginProcessor.cpp
        PluginProcessor.h
        PluginEditor.cpp
        PluginEditor.h
)

target_compile_features(ChordFlowMelody PRIVATE cxx_std_17)

target_link_libraries(ChordFlowMelody
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
        juce::juce_gui_extra

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(ChordFlowMelody)