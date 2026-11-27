# Minimum required version of CMake
cmake_minimum_required(VERSION 3.15)

# Set the executable/plugin name
set(EXEC_NAME Flora)

# Set the project name and version
project(${EXEC_NAME} VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# JUCE path
add_subdirectory("JUCE" "${CMAKE_BINARY_DIR}/JUCE")

# Include your image files in BinaryData (must be declared *before* linking)
juce_add_binary_data(BinaryData SOURCES
    Images/BG1.png
    Images/rotaryknob.png
)

# Create the plugin
juce_add_plugin(${EXEC_NAME}
    FORMATS AU VST3 Standalone
    COMPANY_NAME "AvantiSingh"
    PLUGIN_NAME "Flora"
    # Fixed IDs so hosts recognise saved sessions from earlier builds.
    # JUCE picks a random code when these are unset.
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Qbjy
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# List source files explicitly
target_sources(${EXEC_NAME}
    PRIVATE
        Source/PluginProcessor.h
        Source/PluginEditor.h
        Source/RotaryKnob.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/RotaryKnob.cpp
)

# Link JUCE modules and BinaryData
target_link_libraries(${EXEC_NAME}
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
        juce::juce_osc
        BinaryData    
)

# Optional definitions
target_compile_definitions(${EXEC_NAME}
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_PLUGINHOST_VST=0
        JUCE_VST2_SDK_ENABLED=0
)

# Auto-generate JuceHeader.h
juce_generate_juce_header(${EXEC_NAME})
