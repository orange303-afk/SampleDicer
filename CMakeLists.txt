cmake_minimum_required(VERSION 3.22)
project(SampleDicer VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  add_subdirectory(JUCE)
else()
  message(FATAL_ERROR "JUCE not found. Clone it into ./JUCE")
endif()
juce_add_plugin(SampleDicer COMPANY_NAME "Sample Dicer" BUNDLE_ID com.sampledicer.plugin
  PLUGIN_MANUFACTURER_CODE SmDc PLUGIN_CODE Sdcr FORMATS AU VST3 Standalone
  PRODUCT_NAME "Sample Dicer" IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(SampleDicer)
target_sources(SampleDicer PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_definitions(SampleDicer PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(SampleDicer PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC
  juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
